#include <windows.h>
#include <cmath>
#include <algorithm>
#include "XInput.h"
#include "IniReader\IniReader.h"
#pragma comment(lib, "XInput.lib")

float SensX, SensY;
float ArduinoData[4] = { 0, 0, 0, 0 }; // Mode, Yaw, Pitch, Roll
float LastArduinoData[4] = { 0, 0, 0, 0 };
float YRPOffset[3] = { 0, 0, 0 };
float DeltaYRP[3] = { 0, 0, 0 };
float LastYRP[3] = { 0, 0, 0 };
float accumulatedX = 0, accumulatedY = 0;
int GameMode = 2;
bool ArduinoWork = false;
XINPUT_STATE pState;

double OffsetYPR(double f, double f2)
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

int main()
{
	SetConsoleTitle("X360Advance");
	
	CIniReader IniFile("X360Advance.ini");
	SensX = IniFile.ReadFloat("Main", "SensX", 35);
	SensY = IniFile.ReadFloat("Main", "SensY", 30);
	bool OnlyTrigger = IniFile.ReadBoolean("Main", "OnlyTrigger", false);
	HANDLE hSerial;

	char sPortName[8];
	sprintf_s(sPortName, "COM%d", IniFile.ReadInteger("Main", "ComPort", 2));

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
			}
		}
	}

	if (ArduinoWork == false)
	{
		printf("There are some problems with COM port\r\n");
	}
	else
	{
		printf("Mouse movement activated\r\nNumpad 5 - reset\r\n");
		printf("Press \"~\" to exit\r\n");
	}

	DWORD bytesRead;
	bool Debug = false;
	BYTE DebugMode = 0;
	float MaxAngle = 0, MinAngle = 0;

	while (ArduinoWork) {
		if ((GetAsyncKeyState(192) & 0x8000) != 0) break; //~

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

		if (ArduinoData[0] == 1)
			GameMode = 0;

		if (ArduinoData[0] == 2)
			GameMode = 1;

		if (ArduinoData[0] == 4)
		{
			GameMode = 2;
			PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
			YRPOffset[0] = LastArduinoData[0];
			YRPOffset[1] = LastArduinoData[2];
			YRPOffset[2] = LastArduinoData[3];
		}

		if (GameMode == 2) {
			DeltaYRP[0] = OffsetYPR(ArduinoData[1], LastYRP[0]) * -1;
			DeltaYRP[2] = OffsetYPR(ArduinoData[3], LastYRP[2]);

			XInputGetState(0, &pState);

			if (pState.Gamepad.bLeftTrigger == 0) {
				if (OnlyTrigger == false)
					MoveMouse(DeltaYRP[0] * SensX, DeltaYRP[2] * SensY);
			}
			else
				MoveMouse(DeltaYRP[0] * SensX, DeltaYRP[2] * SensY);

			LastYRP[0] = ArduinoData[1];
			LastYRP[2] = ArduinoData[3];
		}

		if ((GetAsyncKeyState(VK_NUMPAD5) & 0x8000) != 0) {
			PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
			//Centering
			YRPOffset[0] = LastArduinoData[0];
			YRPOffset[1] = LastArduinoData[2];
			YRPOffset[2] = LastArduinoData[3];
		}

		if ((GetAsyncKeyState(VK_NUMPAD0) & 0x8000) != 0) //Debug
			Debug = !Debug;
			
		if (Debug) {

			if ((GetAsyncKeyState(VK_NUMPAD1) & 0x8000) != 0)
			{
				DebugMode = 1;
				MaxAngle = 0;
				MinAngle = 0;
			}

			if ((GetAsyncKeyState(VK_NUMPAD2) & 0x8000) != 0)
			{
				DebugMode = 2;
				MaxAngle = 0;
				MinAngle = 0;
			}
			
			if ((GetAsyncKeyState(VK_NUMPAD3) & 0x8000) != 0)
			{
				DebugMode = 3;
				MaxAngle = 0;
				MinAngle = 0;
			}

			if (DebugMode == 0)
				printf("%7.2f \t %7.2f \t %7.2f \t %7.2f\r\n", ArduinoData[0], ArduinoData[1], ArduinoData[2], ArduinoData[3]);

			if (DebugMode > 0 && DebugMode < 4)
			{
				if (MaxAngle < ArduinoData[DebugMode])
					MaxAngle = ArduinoData[DebugMode];

				if (MinAngle > ArduinoData[DebugMode])
					MinAngle = ArduinoData[DebugMode];

				printf("YPR index=%d\tAngle=%7.2f\tMax=%7.2f\tMin=%7.2f\r\n", DebugMode, ArduinoData[DebugMode], MaxAngle, MinAngle);
			}

		}

		if (bytesRead == 0) Sleep(1); // Don't overload CPU
	}
	
	CloseHandle(hSerial);
	system("pause");
	return 0;
}

