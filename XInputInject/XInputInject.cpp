#include <windows.h>
#include <thread>
#include <math.h>
#include <atlstr.h> 
#include "MinHook.h"

#if defined _M_X64
#pragma comment(lib, "libMinHook-x64-v141-md.lib")
#elif defined _M_IX86
#pragma comment(lib, "libMinHook-x86-v141-md.lib")
#endif

#pragma comment(lib, "winmm.lib")

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


typedef DWORD(WINAPI *XINPUTGETSTATE)(DWORD, XINPUT_STATE*);

// Pointer for calling original
static XINPUTGETSTATE hookedXInputGetState = nullptr;

// wrapper for easier setting up hooks for MinHook
template <typename T>
inline MH_STATUS MH_CreateHookEx(LPVOID pTarget, LPVOID pDetour, T** ppOriginal)
{
	return MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}

template <typename T>
inline MH_STATUS MH_CreateHookApiEx(LPCWSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, T** ppOriginal)
{
	return MH_CreateHookApi(pszModule, pszProcName, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}

bool ArduinoInit = false, ArduinoWork = false;
std::thread *pArduinothread = NULL;
HANDLE hSerial;
float ArduinoData[4] = { 0, 0, 0, 0 }; //Mode, Yaw, Pitch, Roll
float LastArduinoData[4] = { 0, 0, 0, 0 };
float YRPOffset[3] = { 0, 0, 0 };
BYTE GameMode = 0;
DWORD WheelAngle, SensX, SensY, TriggerSens, OnlyTrigger;
float accumulatedX = 0, accumulatedY = 0;
DWORD WorkStatus = 0;

void Centering()
{
	YRPOffset[0] = ArduinoData[1];
	YRPOffset[1] = ArduinoData[2];
	YRPOffset[2] = ArduinoData[3];
}

bool CorrectAngleValue(float Value)
{
	if (Value > -180 && Value < 180)
		return true;
	else
		return false;
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
		if (ArduinoData[0] == 6)
		{
			GameMode = 3;
			PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
			Centering();
		}
		
		//Sleep(1); //Don't overload CPU
	}
}

void ArduinoStart()
{
	TCHAR HardwareInsertWav[MAX_PATH] = { 0 };
	TCHAR HardwareFailWav[MAX_PATH] = { 0 };
	GetSystemWindowsDirectory(HardwareInsertWav, sizeof(HardwareInsertWav));
	_tcscpy_s(HardwareFailWav, sizeof(HardwareInsertWav), HardwareInsertWav);
	_tcscat_s(HardwareInsertWav, sizeof(HardwareInsertWav), _T("\\Media\\Windows Hardware Insert.wav"));
	_tcscat_s(HardwareFailWav, sizeof(HardwareFailWav), _T("\\Media\\Windows Hardware Fail.wav")); 

	CRegKey key;
	DWORD PortNumber = 0;

	LONG status = key.Open(HKEY_CURRENT_USER, _T("Software\\r57zone\\X360Advance"));
	if (status == ERROR_SUCCESS)
	{

		key.QueryDWORDValue(_T("Port"), PortNumber);
		
		key.QueryDWORDValue(_T("WheelAngle"), WheelAngle); //def 75
		key.QueryDWORDValue(_T("SensX"), SensX); //def 35
		key.QueryDWORDValue(_T("SensY"), SensY); //def 30
		key.QueryDWORDValue(_T("TriggerSens"), TriggerSens);
		TriggerSens = TriggerSens / 10.0f;
		key.QueryDWORDValue(_T("OnlyTrigger"), OnlyTrigger);
	}
	key.Close();

	char sPortName[8];
	sprintf_s(sPortName, "COM%d", PortNumber);

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
				WorkStatus++;
				if (WorkStatus == 3)
					PlaySound(HardwareInsertWav, NULL, SND_ASYNC); //Alarm04.wav
				ArduinoWork = true;
				PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
				pArduinothread = new std::thread(ArduinoRead);
			}
		}
	}

	if (WorkStatus < 3)
		PlaySound(HardwareFailWav, NULL, SND_ASYNC);
}

SHORT ToLeftStick(double Value)
{
	int MyValue = round((32767 / WheelAngle) * Value);
	if (MyValue < -32767) MyValue = -32767;
	if (MyValue > 32767) MyValue = 32767;
	return MyValue;
}

SHORT ThumbFix(double Value)
{
	int MyValue = round(Value);
	if (MyValue > 32767) MyValue = 32767;
	if (MyValue < -32767) MyValue = -32767;
	return MyValue;
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

//Implementation from https://github.com/JibbSmart/JoyShockMapper/blob/master/JoyShockMapper/src/win32/InputHelpers.cpp
void MoveMouse(float x, float y) {
	accumulatedX += x;
	accumulatedY += y;

	int applicableX = (int)accumulatedX;
	int applicableY = (int)accumulatedY;

	accumulatedX -= applicableX;
	accumulatedY -= applicableY;

	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.mouseData = 0;
	input.mi.time = 0;
	input.mi.dx = applicableX;
	input.mi.dy = applicableY;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	SendInput(1, &input, sizeof(input));
}

//Own GetState
DWORD WINAPI detourXInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	if (ArduinoInit == false) {
		ArduinoInit = true;
		ArduinoStart();
	}

	//ZeroMemory(pState, sizeof(XINPUT_STATE));

	// first call the original function
	DWORD toReturn = hookedXInputGetState(dwUserIndex, pState);

	//Bugs: Crysis 2 is having problems, although it seemed to work before.

	if (toReturn == ERROR_SUCCESS && ArduinoWork)
		switch (GameMode)
		{
			case 1: //Wheel 
			{
				pState->Gamepad.sThumbLX = ToLeftStick(OffsetYPR(ArduinoData[1], YRPOffset[0])) * -1;
				break;
			}
			case 2:	//FPS
			{
				float NewX = OffsetYPR(ArduinoData[1], YRPOffset[0]) * -1;
				float NewY = OffsetYPR(ArduinoData[3], YRPOffset[2]);

				if (pState->Gamepad.bLeftTrigger == 0) {
					if (OnlyTrigger == false)
						MoveMouse(NewX * SensX, NewY * SensY);
				} else
					MoveMouse(NewX * SensX * TriggerSens, NewY * SensY * TriggerSens);

				Centering();

				break;
			}
			case 3:	//Fully emulation, experimental
			{
				pState->Gamepad.sThumbRX = ThumbFix(OffsetYPR(ArduinoData[1], YRPOffset[0]) * -750);
				pState->Gamepad.sThumbRY = ThumbFix(OffsetYPR(ArduinoData[3], YRPOffset[2]) * -750);

				Centering();

				break;
			}
			/*case 4:	//FPS, gyroscope offset, experimental
			{
				pState->Gamepad.sThumbRX = ThumbFix(myPState.Gamepad.sThumbRX + OffsetYPR(ArduinoData[1], YRPOffset[0]) * -182 * StickSensX); //StickSensX - 9
				pState->Gamepad.sThumbRY = ThumbFix(myPState.Gamepad.sThumbRY + OffsetYPR(ArduinoData[3], YRPOffset[2]) * -182 * StickSensY); //StickSensX - 7
			}*/

		}

	return toReturn;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call){
		case DLL_PROCESS_ATTACH:
		{
			//MessageBox(0, "ATTACH", "XINPUT", MB_OK);
			if (MH_Initialize() == MH_OK) {

				//1.0
				if (MH_CreateHookApiEx(L"XINPUT9_1_0", "XInputGetStateEx", &detourXInputGetState, &hookedXInputGetState) == MH_OK) //Ex! - Bulletstorm (2011)
					WorkStatus++;

				//1_1
				if (hookedXInputGetState == nullptr)
					if (MH_CreateHookApiEx(L"XINPUT_1_1", "XInputGetState", &detourXInputGetState, &hookedXInputGetState) == MH_OK) //Ex?
						WorkStatus++;

				//1_2
				if (hookedXInputGetState == nullptr)
					if (MH_CreateHookApiEx(L"XINPUT_1_2", "XInputGetState", &detourXInputGetState, &hookedXInputGetState) == MH_OK) //Ex?
						WorkStatus++;

				//1_3
				if (hookedXInputGetState == nullptr)
					if (MH_CreateHookApiEx(L"XINPUT1_3", "XInputGetState", &detourXInputGetState, &hookedXInputGetState) == MH_OK)
						WorkStatus++;

				//1_4
				if (hookedXInputGetState == nullptr)
					if (MH_CreateHookApiEx(L"XINPUT1_4", "XInputGetState", &detourXInputGetState, &hookedXInputGetState) == MH_OK)
						WorkStatus++;

				//if (MH_EnableHook(&detourXInputGetState) == MH_OK) //Not working
				if (MH_EnableHook(MH_ALL_HOOKS) == MH_OK)
					WorkStatus++;
			}

			break;
		}

		/*case DLL_THREAD_ATTACH:
		{
			break;
		}

		case DLL_THREAD_DETACH:
		{
			break;
		}*/

		case DLL_PROCESS_DETACH:
		{
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
			MH_DisableHook(&detourXInputGetState);
			MH_Uninitialize();
			break;
		}
	}
	return true;
}
