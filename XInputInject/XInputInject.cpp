#include <windows.h>
#include <thread>
#include <math.h>
#include <atlstr.h> 
#include "MinHook.h"
#include <iostream>
#include <algorithm>
#include <mmsystem.h>

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

// Motion tracker X360Advance
bool ArduinoInit = false, ArduinosWork = false, ArduinoWork1 = false;
std::thread *pArduinothread = NULL;
HANDLE hSerial;
float ArduinoData[4] = { 0, 0, 0, 0 }; //Mode, Yaw, Pitch, Roll
float LastArduinoData[4] = { 0, 0, 0, 0 };
float YRPOffset[3] = { 0, 0, 0 };
float DeltaYRP[3] = { 0, 0, 0 };
float LastYRP[3] = { 0, 0, 0 };
BYTE GameMode = 0;
DWORD WheelAngle, SensX, SensY, TriggerSens, OnlyTrigger;//, JoyMouse = true;
float accumulatedX = 0, accumulatedY = 0;
DWORD WorkStatus = 0;

// External pedals
JOYINFOEX ExternalPedalsJoyInfo;
JOYCAPS ExternalPedalsJoyCaps;
int ExternalPedalsJoyIndex = JOYSTICKID1;
std::thread *pArduinothread2 = NULL;
bool ExternalPedalsArduinoConnected = false;
bool ExternalPedalsDInputConnected = false;
HANDLE hSerial2;
float PedalsValues[2];

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

	while (ArduinoWork1) {
		ReadFile(hSerial, &ArduinoData, sizeof(ArduinoData), &bytesRead, 0);

		// Filter incorrect values
		if (CorrectAngleValue(ArduinoData[1]) == false || CorrectAngleValue(ArduinoData[2]) == false || CorrectAngleValue(ArduinoData[3]) == false)
		{
			//Last correct values
			ArduinoData[0] = LastArduinoData[0];
			ArduinoData[1] = LastArduinoData[1];
			ArduinoData[2] = LastArduinoData[2];
			ArduinoData[3] = LastArduinoData[3];

			PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
		}

		// Save last correct values
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

		if (bytesRead == 0) Sleep(1); // Don't overload CPU
	}
}

void ArduinoRead2()
{
	DWORD bytesRead;

	while (ExternalPedalsArduinoConnected) {
		ReadFile(hSerial2, &PedalsValues, sizeof(PedalsValues), &bytesRead, 0);

		if (PedalsValues[0] > 1.0 || PedalsValues[0] < 0 || PedalsValues[1] > 1.0 || PedalsValues[1] < 0)
		{
			PedalsValues[0] = 0;
			PedalsValues[1] = 0;

			PurgeComm(hSerial2, PURGE_TXCLEAR | PURGE_RXCLEAR);
		}

		if (bytesRead == 0) Sleep(1); // Don't overload CPU
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
	DWORD ExternalPedalsPortNumber = 0;

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
		//key.QueryDWORDValue(_T("JoyMouse"), JoyMouse);

		key.QueryDWORDValue(_T("ExternalPedalsPort"), ExternalPedalsPortNumber);
	}
	key.Close();

	char sPortName[32];
	sprintf_s(sPortName, "\\\\.\\COM%d", PortNumber);

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
				ArduinoWork1 = true;
				PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
				pArduinothread = new std::thread(ArduinoRead);
			}
		}
	}

	char sPortName2[32];
	sprintf_s(sPortName2, "\\\\.\\COM%d", ExternalPedalsPortNumber);
	hSerial2 = ::CreateFile(sPortName2, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hSerial2 != INVALID_HANDLE_VALUE && GetLastError() != ERROR_FILE_NOT_FOUND) {

		DCB dcbSerialParams = { 0 };
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

		if (GetCommState(hSerial2, &dcbSerialParams))
		{
			dcbSerialParams.BaudRate = CBR_115200;
			dcbSerialParams.ByteSize = 8;
			dcbSerialParams.StopBits = ONESTOPBIT;
			dcbSerialParams.Parity = NOPARITY;

			if (SetCommState(hSerial2, &dcbSerialParams))
			{
				WorkStatus++;
				ExternalPedalsArduinoConnected = true;
				PurgeComm(hSerial2, PURGE_TXCLEAR | PURGE_RXCLEAR);
				pArduinothread2 = new std::thread(ArduinoRead2);
			}
		}
	}

	if (WorkStatus < 3)
		PlaySound(HardwareFailWav, NULL, SND_ASYNC);
	else if (WorkStatus >= 3)
		PlaySound(HardwareInsertWav, NULL, SND_ASYNC); //Alarm04.wav
}

SHORT ToLeftStick(double Value)
{
	int MyValue = trunc((32767 / WheelAngle) * Value);
	if (MyValue < -32767)
		MyValue = -32767;
	else if (MyValue > 32767)
		MyValue = 32767;
	return MyValue;
}

SHORT ThumbFix(double Value)
{
	int MyValue = trunc(Value);
	if (MyValue > 32767)
		MyValue = 32767;
	else if (MyValue < -32767)
		MyValue = -32767;
	return MyValue;
}

double RadToDeg(double Rad)
{
	return Rad / 3.14159265358979323846 * 180.0;
}

double DegToRad(double Deg)
{
	return Deg * 3.14159265358979323846 / 180.0;
}

double OffsetYPR(float Angle1, float Angle2)
{
	Angle1 -= Angle2;
	if (Angle1 < -180)
		Angle1 += 360;
	else if (Angle1 > 180)
		Angle1 -= 360;
	return Angle1;
}

// Implementation from https://github.com/JibbSmart/JoyShockMapper/blob/master/JoyShockMapper/src/win32/InputHelpers.cpp
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

float ClampFloat(float Value, float Min, float Max)
{
	if (Value > Max)
		Value = Max;
	else if (Value < Min)
		Value = Min;
	return Value;
}

void ExternalPedalsDInputSearch() {
	if (joyGetPosEx(JOYSTICKID1, &ExternalPedalsJoyInfo) == JOYERR_NOERROR &&
		joyGetDevCaps(JOYSTICKID1, &ExternalPedalsJoyCaps, sizeof(ExternalPedalsJoyCaps)) == JOYERR_NOERROR &&
		ExternalPedalsJoyCaps.wNumButtons == 16) { // DualSense - 15, DigiJoy - 16
		ExternalPedalsJoyIndex = JOYSTICKID1;
		ExternalPedalsDInputConnected = true;
	}
	else if (joyGetPosEx(JOYSTICKID2, &ExternalPedalsJoyInfo) == JOYERR_NOERROR &&
		joyGetDevCaps(JOYSTICKID2, &ExternalPedalsJoyCaps, sizeof(ExternalPedalsJoyCaps)) == JOYERR_NOERROR &&
		ExternalPedalsJoyCaps.wNumButtons == 16) {
		ExternalPedalsJoyIndex = JOYSTICKID2;
		ExternalPedalsDInputConnected = true;
	}
	else
		ExternalPedalsDInputConnected = false;
}

//Own GetState
DWORD WINAPI detourXInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	if (ArduinoInit == false) {
		ArduinoInit = true;
		ArduinoStart();
		ExternalPedalsJoyInfo.dwFlags = JOY_RETURNALL;
		ExternalPedalsJoyInfo.dwSize = sizeof(ExternalPedalsJoyInfo);
		ExternalPedalsDInputSearch();
		//AllocConsole();
		//FILE* fp;
		//freopen_s(&fp, "CONOUT$", "w", stdout);
	}

	// ZeroMemory(pState, sizeof(XINPUT_STATE));

	// first call the original function
	DWORD toReturn = hookedXInputGetState(dwUserIndex, pState);

	// Crysis 2 reads the state incorrectly, so there is a separate library for it.

	if (toReturn == ERROR_SUCCESS && ArduinoWork1) {
		if (GameMode == 1)
				pState->Gamepad.sThumbLX = ToLeftStick(OffsetYPR(ArduinoData[1], YRPOffset[0])) * -1;
		
		else if (GameMode == 2) {
				DeltaYRP[0] = OffsetYPR(ArduinoData[1], LastYRP[0]) * -1;
				//DeltaYRP[1] = OffsetYPR(ArduinoData[2], LastYRP[1]);
				DeltaYRP[2] = OffsetYPR(ArduinoData[3], LastYRP[2]);

				if (pState->Gamepad.bLeftTrigger == 0) {
					if (OnlyTrigger == false)
						//if (JoyMouse == false)
							MoveMouse(DeltaYRP[0] * SensX, DeltaYRP[2] * SensY);
						//else {
							//pState->Gamepad.sThumbRX = std::clamp((int)(ClampFloat(RadToDeg(DeltaYRP[0]) * SensX, -1, 1) * 32767 + pState->Gamepad.sThumbRX), -32767, 32767);
							//pState->Gamepad.sThumbRY = std::clamp((int)(ClampFloat(-(RadToDeg(DeltaYRP[2])) * SensY, -1, 1) * 32767 + pState->Gamepad.sThumbRY), -32767, 32767);
							//pState->Gamepad.sThumbRX = std::clamp((int)(DeltaYRP[0] * SensX * 32767), -32767, 32767);
							//pState->Gamepad.sThumbRY = std::clamp((int)(-DeltaYRP[2] * SensY * 32767), -32767, 32767);


							//pState->Gamepad.sThumbRX = ThumbFix(OffsetYPR(ArduinoData[1], YRPOffset[0]) * -750 + pState->Gamepad.sThumbRX);		
							//pState->Gamepad.sThumbRY = ThumbFix(OffsetYPR(ArduinoData[3], YRPOffset[2]) * -750 + pState->Gamepad.sThumbRY);

							
							// Слишком быстро, не успевает, нужно иначе делать
							pState->Gamepad.sThumbRX = ThumbFix(-(DegToRad(DeltaYRP[0])) * 20870 * SensX * TriggerSens + pState->Gamepad.sThumbRX); // 20870 = 32767 / 1,57 (половина радианов)
							pState->Gamepad.sThumbRY = ThumbFix(DegToRad(DeltaYRP[2]) * 20870 * SensY * TriggerSens + pState->Gamepad.sThumbRY);
							
							//std::cout << DegToRad(DeltaYRP[0]) * 20870 * SensX << std::endl;
							//system("cls");

							//Centering();
						
							//YRPOffset[0] = ArduinoData[1];
							//YRPOffset[2] = ArduinoData[3];
						//}

				} else
					//if (JoyMouse == false)
						MoveMouse(DeltaYRP[0] * SensX * TriggerSens, DeltaYRP[2] * SensY * TriggerSens);
					//else {
						//pState->Gamepad.sThumbRX = std::clamp((int)(ClampFloat(DeltaYRP[0] * SensX * TriggerSens, -1, 1) * 32767 + pState->Gamepad.sThumbRX), -32767, 32767);
						//pState->Gamepad.sThumbRY = std::clamp((int)(ClampFloat(-(DeltaYRP[2]) * SensY * TriggerSens, -1, 1) * 32767 + pState->Gamepad.sThumbRY), -32767, 32767);

						//pState->Gamepad.sThumbRX = ThumbFix(-(DegToRad(DeltaYRP[0])) * 20870 * SensX * TriggerSens + pState->Gamepad.sThumbRX); // 20870 = 32767 / 1,57 (половина радианов)
						//pState->Gamepad.sThumbRY = ThumbFix(DegToRad(DeltaYRP[2]) * 20870 * SensY * TriggerSens + pState->Gamepad.sThumbRY);
						
					//}



				LastYRP[0] = ArduinoData[1];
				//LastYRP[1] = ArduinoData[2];
				LastYRP[2] = ArduinoData[3];

				//pState->Gamepad.sThumbRX = ThumbFix(OffsetYPR(ArduinoData[1], YRPOffset[0]) * -750 + pState->Gamepad.sThumbRX);
				//pState->Gamepad.sThumbRY = ThumbFix(OffsetYPR(ArduinoData[3], YRPOffset[2]) * -750 + pState->Gamepad.sThumbRY);
				//Centering();
		}

		if (ExternalPedalsDInputConnected) {
			if (joyGetPosEx(ExternalPedalsJoyIndex, &ExternalPedalsJoyInfo) == JOYERR_NOERROR) {
				if (pState->Gamepad.bLeftTrigger == 0)
					pState->Gamepad.bLeftTrigger = ExternalPedalsJoyInfo.dwVpos / 256;
				if(pState->Gamepad.bRightTrigger == 0)
					pState->Gamepad.bRightTrigger = ExternalPedalsJoyInfo.dwUpos / 256;
			} else
				ExternalPedalsDInputConnected = false;
		} else if (ExternalPedalsArduinoConnected) {
			if (pState->Gamepad.bLeftTrigger == 0)
				pState->Gamepad.bLeftTrigger = PedalsValues[0] * 255;
			if (pState->Gamepad.bRightTrigger == 0)
				pState->Gamepad.bRightTrigger = PedalsValues[1] * 255;
		}
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
			// Injection trick taken here https://github.com/FransBouma/InjectableGenericCameraSystem
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
			if (ArduinoWork1) {
				ArduinoWork1 = false;
				if (pArduinothread)
				{
					pArduinothread->join();
					delete pArduinothread;
					pArduinothread = nullptr;
				}
				CloseHandle(hSerial);
			}
			if (ExternalPedalsArduinoConnected) {
				ExternalPedalsArduinoConnected = false;
				if (pArduinothread2)
				{
					pArduinothread2->join();
					delete pArduinothread2;
					pArduinothread2 = nullptr;
				}
				CloseHandle(hSerial2);
			}
			MH_DisableHook(&detourXInputGetState);
			MH_Uninitialize();
			break;
		}
	}
	return true;
}
