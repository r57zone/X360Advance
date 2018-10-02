#include <thread>
#include <windows.h>
#include <math.h>
#include <atlstr.h> 
#include "IniReader\IniReader.h"

#define DLLEXPORT extern "C" __declspec(dllexport)

#define XINPUT_GAMEPAD_DPAD_UP          0x0001
#define XINPUT_GAMEPAD_DPAD_DOWN        0x0002
#define XINPUT_GAMEPAD_DPAD_LEFT        0x0004
#define XINPUT_GAMEPAD_DPAD_RIGHT       0x0008
#define XINPUT_GAMEPAD_START            0x0010
#define XINPUT_GAMEPAD_BACK             0x0020
#define XINPUT_GAMEPAD_LEFT_THUMB       0x0040
#define XINPUT_GAMEPAD_RIGHT_THUMB      0x0080
#define XINPUT_GAMEPAD_LEFT_SHOULDER    0x0100
#define XINPUT_GAMEPAD_RIGHT_SHOULDER   0x0200
#define XINPUT_GAMEPAD_A                0x1000
#define XINPUT_GAMEPAD_B                0x2000
#define XINPUT_GAMEPAD_X                0x4000
#define XINPUT_GAMEPAD_Y				0x8000

#define BATTERY_TYPE_DISCONNECTED		0x00

#define XUSER_MAX_COUNT                 4
#define XUSER_INDEX_ANY					0x000000FF

#define ERROR_DEVICE_NOT_CONNECTED		1167
#define ERROR_SUCCESS					0

//
// Structures used by XInput APIs
//
typedef struct _XINPUT_GAMEPAD
{
	WORD                                wButtons;
	BYTE                                bRightTrigger;
	BYTE                                bLeftTrigger;
	SHORT                               sThumbLX;
	SHORT                               sThumbLY;
	SHORT                               sThumbRX;
	SHORT                               sThumbRY;
} XINPUT_GAMEPAD, *PXINPUT_GAMEPAD;

typedef struct _XINPUT_STATE
{
	DWORD                               dwPacketNumber;
	XINPUT_GAMEPAD                      Gamepad;
} XINPUT_STATE, *PXINPUT_STATE;

typedef struct _XINPUT_VIBRATION
{
	WORD                                wLeftMotorSpeed;
	WORD                                wRightMotorSpeed;
} XINPUT_VIBRATION, *PXINPUT_VIBRATION;

typedef struct _XINPUT_CAPABILITIES
{
	BYTE                                Type;
	BYTE                                SubType;
	WORD                                Flags;
	XINPUT_GAMEPAD                      Gamepad;
	XINPUT_VIBRATION                    Vibration;
} XINPUT_CAPABILITIES, *PXINPUT_CAPABILITIES;

typedef struct _XINPUT_BATTERY_INFORMATION
{
	BYTE BatteryType;
	BYTE BatteryLevel;
} XINPUT_BATTERY_INFORMATION, *PXINPUT_BATTERY_INFORMATION;

typedef struct _XINPUT_KEYSTROKE
{
	WORD    VirtualKey;
	WCHAR   Unicode;
	WORD    Flags;
	BYTE    UserIndex;
	BYTE    HidCode;
} XINPUT_KEYSTROKE, *PXINPUT_KEYSTROKE;

typedef DWORD(__stdcall *_XInputGetState)(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE *pState);
typedef DWORD(__stdcall *_XInputSetState)(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION *pVibration);

_XInputGetState MyXInputGetState;
_XInputSetState MyXInputSetState;

HMODULE hDll;

bool ArduinoInit = false, ArduinoWork = false;
std::thread *pArduinothread = NULL;
HANDLE hSerial;
float ArduinoYPR[3], YRPOffset[3], test;

void ArduinoRead()
{
	DWORD bytesRead;

	memset(&ArduinoYPR[3], 0, sizeof(ArduinoYPR[3]));

	while (ArduinoWork) {
		ReadFile(hSerial, &ArduinoYPR, sizeof(ArduinoYPR), &bytesRead, 0);
	}	
}

BYTE GameMode = 0;
double WheelAngle, RThumbSensX, RThumbSensY;
_XINPUT_STATE myPState;

SHORT ToLeftStick(double Data)
{
	int MyData = round((32767 / WheelAngle) * Data); 
	if (MyData < -32767) MyData = -32767;
	if (MyData > 32767) MyData = 32767;
	return MyData;
}

SHORT ThumbFix(double Data)
{
	int MyData = round(Data);
	if (MyData > 32767) MyData = 32767;
	if (MyData < -32767) MyData = -32767;
	return MyData;
}

DLLEXPORT BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	//case DLL_PROCESS_ATTACH:
	//case DLL_THREAD_ATTACH:
	//case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		if (hDll != NULL) {
			FreeLibrary(hDll);
			hDll = nullptr;
		}
		if (ArduinoWork) {
			ArduinoWork = false;
			if (pArduinothread) {
				pArduinothread->join();
				delete pArduinothread;
				pArduinothread = nullptr;
			}
		}
		break;
	}
	return TRUE;
}

double MyOffset(float f, float f2)
{
	return fmod(f - f2, 180);
}

void ArduinoStart() {
	CIniReader IniFile("X360Advance.ini");
	WheelAngle = IniFile.ReadFloat("Main", "WheelAngle", 80);
	RThumbSensX = IniFile.ReadFloat("Main", "RThumbSensX", -1);
	RThumbSensY = IniFile.ReadFloat("Main", "RThumbSensY", -1);

	hDll = LoadLibrary(_T("C:\\Windows\\System32\\xinput1_3.dll"));

	if (hDll != NULL) {

		MyXInputGetState = (_XInputGetState)GetProcAddress(hDll, "XInputGetState");
		MyXInputSetState = (_XInputSetState)GetProcAddress(hDll, "XInputSetState");

		if (MyXInputGetState == NULL || MyXInputSetState == NULL)
			hDll = NULL;

	}

	CString sPortName;
	sPortName.Format(_T("COM%d"), IniFile.ReadInteger("Main", "ComPort", 3));

	hSerial = ::CreateFile(sPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hSerial != INVALID_HANDLE_VALUE && GetLastError() != ERROR_FILE_NOT_FOUND) {

		DCB dcbSerialParams = { 0 };
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

		if (GetCommState(hSerial, &dcbSerialParams))
		{
			dcbSerialParams.BaudRate = CBR_115200;
			dcbSerialParams.ByteSize = 8;
			dcbSerialParams.StopBits = ONESTOPBIT;
			dcbSerialParams.Parity = NOPARITY;

			if (SetCommState(hSerial, &dcbSerialParams))
			{
				ArduinoWork = true;
				pArduinothread = new std::thread(ArduinoRead);
			}
		}
	}
}

DLLEXPORT DWORD WINAPI XInputGetState(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE *pState)
{
	if (ArduinoInit == false) {
		ArduinoInit = true;
		ArduinoStart();
	}
	
	pState->Gamepad.wButtons = 0;
	pState->Gamepad.bRightTrigger = 0;
	pState->Gamepad.bLeftTrigger = 0;
	pState->Gamepad.sThumbLX = 0;
	pState->Gamepad.sThumbLY = 0;
	pState->Gamepad.sThumbRX = 0;
	pState->Gamepad.sThumbRY = 0;

	DWORD myStatus = ERROR_DEVICE_NOT_CONNECTED;
	if (hDll != NULL)
		myStatus = MyXInputGetState(dwUserIndex, &myPState);
	
	if (myStatus == ERROR_SUCCESS) {
		pState->Gamepad.bRightTrigger = myPState.Gamepad.bRightTrigger;
		pState->Gamepad.bLeftTrigger = myPState.Gamepad.bLeftTrigger;
		pState->Gamepad.wButtons = myPState.Gamepad.wButtons;

		//FPS
		if ((myPState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
			&& (myPState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
			&& (myPState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)) {
			GameMode = 1;
			pState->Gamepad.wButtons = 0; //Не нужно нажимать кнопки в игре при смене режима
		}

		//Wheel
		if ((myPState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
			&& (myPState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
			&& (myPState.Gamepad.wButtons & XINPUT_GAMEPAD_START)) {
			GameMode = 2;
			pState->Gamepad.wButtons = 0;
		}

		//Default
		if ((myPState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
			&& (myPState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
			&& (myPState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)
			&& (myPState.Gamepad.wButtons & XINPUT_GAMEPAD_START)) {
			GameMode = 0;
			pState->Gamepad.wButtons = 0;
		}

		//Centring
		if ((myPState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
			&& (myPState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
			&& (myPState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)) {

			YRPOffset[0] = ArduinoYPR[0];
			YRPOffset[1] = ArduinoYPR[1];
			YRPOffset[2] = ArduinoYPR[2];

			pState->Gamepad.wButtons = 0;
		}

		switch (GameMode)
		{
		case 0: //Default
			{
				pState->Gamepad.sThumbLX = myPState.Gamepad.sThumbLX;
				pState->Gamepad.sThumbLY = myPState.Gamepad.sThumbLY;
				pState->Gamepad.sThumbRX = myPState.Gamepad.sThumbRX;
				pState->Gamepad.sThumbRY = myPState.Gamepad.sThumbRY;
			}
			break;
		case 1: //FPS
			{
				pState->Gamepad.sThumbLX = myPState.Gamepad.sThumbLX;
				pState->Gamepad.sThumbLY = myPState.Gamepad.sThumbLY;
				pState->Gamepad.sThumbRX = ThumbFix(myPState.Gamepad.sThumbRX + MyOffset(ArduinoYPR[2], YRPOffset[2]) * RThumbSensX);
				pState->Gamepad.sThumbRY = ThumbFix(myPState.Gamepad.sThumbRY + MyOffset(ArduinoYPR[1], YRPOffset[1]) * RThumbSensY);
			}
			break;
		case 2:	//Wheel
			{
				pState->Gamepad.sThumbLX = ToLeftStick(MyOffset(ArduinoYPR[2], YRPOffset[2]) * -1);
				pState->Gamepad.sThumbLY = myPState.Gamepad.sThumbLY;
				pState->Gamepad.sThumbRX = myPState.Gamepad.sThumbRX;
				pState->Gamepad.sThumbRY = myPState.Gamepad.sThumbRY;
			}
			break;
		}
	}


	pState->dwPacketNumber = GetTickCount();

	return myStatus;
}

DLLEXPORT DWORD WINAPI XInputSetState(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION *pVibration) 
{
	DWORD myStatus = ERROR_DEVICE_NOT_CONNECTED;
	if (hDll != NULL)
		myStatus = MyXInputSetState(dwUserIndex, pVibration);
	return myStatus;
}


DLLEXPORT DWORD WINAPI XInputGetCapabilities(_In_ DWORD dwUserIndex, _In_ DWORD dwFlags, _Out_ XINPUT_CAPABILITIES *pCapabilities) 
{
	if (dwUserIndex == 0) {
		return ERROR_SUCCESS;
	}
	else
	{
		return ERROR_DEVICE_NOT_CONNECTED;
	}
}

DLLEXPORT void WINAPI XInputEnable(_In_ BOOL enable)
{

}

DLLEXPORT DWORD WINAPI XInputGetDSoundAudioDeviceGuids(DWORD dwUserIndex, GUID* pDSoundRenderGuid, GUID* pDSoundCaptureGuid)
{
	if (dwUserIndex == 0) {
		return ERROR_SUCCESS;
	}
	else
	{
		return ERROR_DEVICE_NOT_CONNECTED;
	}
}

DLLEXPORT DWORD WINAPI XInputGetBatteryInformation(_In_ DWORD dwUserIndex, _In_ BYTE devType, _Out_ XINPUT_BATTERY_INFORMATION *pBatteryInformation)
{
	if (dwUserIndex == 0) {
		return ERROR_SUCCESS;
	}
	else
	{
		return ERROR_DEVICE_NOT_CONNECTED;
	}
}

DLLEXPORT DWORD WINAPI XInputGetKeystroke(DWORD dwUserIndex, DWORD dwReserved, PXINPUT_KEYSTROKE pKeystroke)
{
	if (dwUserIndex == 0) {
		return ERROR_SUCCESS;
	}
	else
	{
		return ERROR_DEVICE_NOT_CONNECTED;
	}
}

DLLEXPORT DWORD WINAPI XInputGetStateEx(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE *pState)
{
	if (dwUserIndex == 0) {
		return ERROR_SUCCESS;
	}
	else
	{
		return ERROR_DEVICE_NOT_CONNECTED;
	}
}

DLLEXPORT DWORD WINAPI XInputWaitForGuideButton(_In_ DWORD dwUserIndex, _In_ DWORD dwFlag, _In_ LPVOID pVoid)
{
	if (dwUserIndex == 0) {
		return ERROR_SUCCESS;
	}
	else
	{
		return ERROR_DEVICE_NOT_CONNECTED;
	}
}

DLLEXPORT DWORD XInputCancelGuideButtonWait(_In_ DWORD dwUserIndex)
{
	if (dwUserIndex == 0) {
		return ERROR_SUCCESS;
	}
	else
	{
		return ERROR_DEVICE_NOT_CONNECTED;
	}
}

DLLEXPORT DWORD XInputPowerOffController(_In_ DWORD dwUserIndex)
{
	if (dwUserIndex == 0) {
		return ERROR_SUCCESS;
	}
	else
	{
		return ERROR_DEVICE_NOT_CONNECTED;
	}
}