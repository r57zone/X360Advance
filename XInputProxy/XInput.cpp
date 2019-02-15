#include "stdafx.h"

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
	BYTE                                bLeftTrigger;
	BYTE                                bRightTrigger;
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
_XINPUT_STATE myPState;

HMODULE hDll;

bool ArduinoInit = false, ArduinoWork = false;
std::thread *pArduinothread = NULL;
HANDLE hSerial;
float ArduinoData[4] = { 0, 0, 0, 0 }; //Mode, Yaw, Pitch, Roll
float LastArduinoData[4] = { 0, 0, 0, 0 };
float YRPOffset[3] = { 0, 0, 0 };
BYTE GameMode = 0;
double WheelAngle, SensX, SensY;//, TriggerSens;
int last_x = 0, last_y = 0;

void Centering()
{
	YRPOffset[0] = ArduinoData[1];
	YRPOffset[1] = ArduinoData[2];
	YRPOffset[2] = ArduinoData[3];
}

bool CorrectAngleValue(float Value)
{
	if (Value > -180 && Value < 180)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ArduinoRead()
{
	DWORD bytesRead;

	while (ArduinoWork) {
		ReadFile(hSerial, &ArduinoData, sizeof(ArduinoData), &bytesRead, 0);

		//Filter incorrect values
		if (CorrectAngleValue(ArduinoData[1]) == false || CorrectAngleValue(ArduinoData[2]) == false || CorrectAngleValue(ArduinoData[3]) == false)
		{
			//Last correct values
			ArduinoData[0] = LastArduinoData[0];
			ArduinoData[1] = LastArduinoData[1];
			ArduinoData[2] = LastArduinoData[2];
			ArduinoData[3] = LastArduinoData[3];

			PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
		}

		//Save last correct values
		if (CorrectAngleValue(ArduinoData[1]) && CorrectAngleValue(ArduinoData[2]) && CorrectAngleValue(ArduinoData[3]))
		{
			LastArduinoData[0] = ArduinoData[0];
			LastArduinoData[1] = ArduinoData[1];
			LastArduinoData[2] = ArduinoData[2];
			LastArduinoData[3] = ArduinoData[3];
		}

		if (ArduinoData[0] == 1) { GameMode = 0; }
		if (ArduinoData[0] == 2)
		{ 
			GameMode = 1;
			PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
			Centering();
		}
		if (ArduinoData[0] == 4)
		{
			GameMode = 2;
			PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
			Centering();
		}


	}
}

void ArduinoStart() {
	CIniReader IniFile("X360Advance.ini");
	WheelAngle = IniFile.ReadFloat("Main", "WheelAngle", 75);
	SensX = IniFile.ReadFloat("Main", "SensX", 4.5);
	SensY = IniFile.ReadFloat("Main", "SensY", 3.5);
	//TriggerSens = IniFile.ReadFloat("Main", "TriggerSens", 0.5);

	TCHAR XInputPath[MAX_PATH] = { 0 };
	GetSystemWindowsDirectory(XInputPath, sizeof(XInputPath));

	_tcscat_s(XInputPath, sizeof(XInputPath), _T("\\System32\\xinput1_3.dll")); //Separate paths for different architectures are not required. Windows does it by itself.

	hDll = LoadLibrary(XInputPath);
	//hDll = LoadLibrary(_T("C:\\Windows\\System32\\xinput1_3.dll"));

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
				PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
				pArduinothread = new std::thread(ArduinoRead);
			}
		}
	}
}

SHORT ToLeftStick(double Value)
{
	int MyValue = round((32767 / WheelAngle) * Value);
	if (MyValue < -32767) MyValue = -32767;
	if (MyValue > 32767) MyValue = 32767;
	return MyValue;
}

/*SHORT ThumbFix(double Value)
{
	int MyValue = round(Value);
	if (MyValue > 32767) MyValue = 32767;
	if (MyValue < -32767) MyValue = -32767;
	return MyValue;
}*/

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
		
			if (ArduinoWork) {
				ArduinoWork = false;
				if (pArduinothread)
				{
					pArduinothread->join();
					delete pArduinothread;
					pArduinothread = nullptr;
				}
			}
			CloseHandle(hSerial);
		}
		break;
	}
	return true;
}

double OffsetYPR(float f, float f2)
{
	f -= f2;
	if (f < -180) {
		f += 360;
	}
	else if (f > 180) {
		f -= 360;
	}

	return f;
}

int MouseGetDelta(int val, int prev) //Implementation from OpenTrack https://github.com/opentrack/opentrack/blob/unstable/proto-mouse/
{
	const int a = std::abs(val - prev), b = std::abs(val + prev);
	if (b < a)
		return val + prev;
	else
		return val - prev;
}

void MouseMove(const double axisX, const double axisY) //Implementation from OpenTrack https://github.com/opentrack/opentrack/blob/unstable/proto-mouse/
{
	int mouse_x = 0, mouse_y = 0;
	
	mouse_x = round(axisX * SensX * 2);
	mouse_y = round(axisY * SensY * 2);

	const int dx = MouseGetDelta(mouse_x, last_x);
	const int dy = MouseGetDelta(mouse_y, last_y);

	last_x = mouse_x;
	last_y = mouse_y;

	if (dx || dy)
	{
		INPUT input;
		input.type = INPUT_MOUSE;
		MOUSEINPUT& mi = input.mi;
		mi = {};
		mi.dx = dx;
		mi.dy = dy;
		mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_MOVE_NOCOALESCE;

		SendInput(1, &input, sizeof(input));
	}
}

DLLEXPORT DWORD WINAPI XInputGetState(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE *pState)
{
	if (ArduinoInit == false) {
		ArduinoInit = true;
		ArduinoStart();
	}

	pState->Gamepad.wButtons = 0;
	pState->Gamepad.bLeftTrigger = 0;
	pState->Gamepad.bRightTrigger = 0;
	pState->Gamepad.sThumbLX = 0;
	pState->Gamepad.sThumbLY = 0;
	pState->Gamepad.sThumbRX = 0;
	pState->Gamepad.sThumbRY = 0;

	DWORD myStatus = ERROR_DEVICE_NOT_CONNECTED;
	if (hDll != NULL)
		myStatus = MyXInputGetState(dwUserIndex, &myPState);

	if (myStatus == ERROR_SUCCESS) {
		pState->Gamepad.bLeftTrigger = myPState.Gamepad.bLeftTrigger;
		pState->Gamepad.bRightTrigger = myPState.Gamepad.bRightTrigger;
		pState->Gamepad.wButtons = myPState.Gamepad.wButtons;

		pState->Gamepad.sThumbLX = myPState.Gamepad.sThumbLX;
		pState->Gamepad.sThumbLY = myPState.Gamepad.sThumbLY;
		pState->Gamepad.sThumbRX = myPState.Gamepad.sThumbRX;
		pState->Gamepad.sThumbRY = myPState.Gamepad.sThumbRY;

		//FPS
		/*if ((myPState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
			&& (myPState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
			&& (myPState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)) {
			GameMode = 1;
			pState->Gamepad.wButtons = 0; //Не нажимать кнопки при смене режима
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

		if (GetAsyncKeyState(VK_NUMPAD1) & 0x8000)
		{
			GameMode = 0;
		}

		if (GetAsyncKeyState(VK_NUMPAD2) & 0x8000)
		{
			GameMode = 1;
			Centering();
		}

		if (GetAsyncKeyState(VK_NUMPAD3) & 0x8000)
		{
			GameMode = 2;
			Centering();
		}*/

		switch (GameMode)
		{
			case 1: //Wheel
			{
				pState->Gamepad.sThumbLX = ToLeftStick(OffsetYPR(ArduinoData[1], YRPOffset[0])) * -1;
				break;
			}
		

			case 2:	//FPS
			{
				//Fully emulation
				//pState->Gamepad.sThumbRX = ThumbFix(OffsetYPR(ArduinoData[1], YRPOffset[0]) * -750);
				//pState->Gamepad.sThumbRY = ThumbFix(OffsetYPR(ArduinoData[3], YRPOffset[2]) * -750);
			
				//Gyroscope offset
				//pState->Gamepad.sThumbRX = ThumbFix(myPState.Gamepad.sThumbRX + OffsetYPR(ArduinoData[1], YRPOffset[0]) * -182 * StickSensX); //StickSensX - 9
				//pState->Gamepad.sThumbRY = ThumbFix(myPState.Gamepad.sThumbRY + OffsetYPR(ArduinoData[3], YRPOffset[2]) * -182 * StickSensY); //StickSensX - 7

				/*if (pState->Gamepad.bLeftTrigger == 0) {
					MouseMove(OffsetYPR(ArduinoData[1], YRPOffset[0]) * -1, OffsetYPR(ArduinoData[3], YRPOffset[2]));
				} else {
					MouseMove(OffsetYPR(ArduinoData[1], YRPOffset[0]) * -1 * TriggerSens, OffsetYPR(ArduinoData[3], YRPOffset[2]) * TriggerSens);
				}*/

				MouseMove(OffsetYPR(ArduinoData[1], YRPOffset[0]) * -1, OffsetYPR(ArduinoData[3], YRPOffset[2]));
				break;
			}
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
