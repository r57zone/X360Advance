#include "stdafx.h"
#include <windows.h>
#include <atlstr.h> 
#include <cmath>
#include <algorithm>
#include "IniReader\IniReader.h"

float SensX, SensY;
float ArduinoData[4] = { 0, 0, 0, 0 }; //Mode, Yaw, Pitch, Roll
float LastArduinoData[4] = { 0, 0, 0, 0 };
float YRPOffset[3] = { 0, 0, 0 };
int last_x = 0, last_y = 0;
int GameMode = 2;
bool ArduinoWork = false;

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

int main()
{
	SetConsoleTitle("X360Advance");
	
	CIniReader IniFile("X360Advance.ini");
	SensX = IniFile.ReadFloat("Main", "SensX", 4.5);
	SensY = IniFile.ReadFloat("Main", "SensY", 3.5);
	HANDLE hSerial;

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
			}
		}
	}

	if (ArduinoWork == false)
	{
		printf("There are some problems with COM port\r\n");
	}
	else
	{
		printf("Mouse movement activated\r\nNumpad 5 - centering\r\n");
	}

	DWORD bytesRead;

	while (ArduinoWork) {
		if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) break;

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

		if (GameMode == 2)
			MouseMove(OffsetYPR(ArduinoData[1], YRPOffset[0]) * -1, OffsetYPR(ArduinoData[3], YRPOffset[2]));


		if ((GetAsyncKeyState(VK_NUMPAD5) & 0x8000) != 0) {
			PurgeComm(hSerial, PURGE_TXCLEAR | PURGE_RXCLEAR);
			//Centering
			YRPOffset[0] = LastArduinoData[0];
			YRPOffset[1] = LastArduinoData[2];
			YRPOffset[2] = LastArduinoData[3];
		}
	}
	
	CloseHandle(hSerial);
	system("pause");
	return 0;
}

