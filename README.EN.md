[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/XInputNext/X360Advance/blob/master/README.md) 
[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/XInputNext/X360Advance/blob/master/README.EN.md) 
# X360Advance (XInput) 
External Arduino Gyroscope for the Xbox gamepad that allows you to use it as a steering wheel, and also help you more accurately aim. There is also a [version for OpenTrack](https://github.com/r57zone/X360Advance).

![](https://user-images.githubusercontent.com/9499881/27588504-749af800-5b59-11e7-92e4-2b3813428281.png)

There are 3 modes of use:

1. FPS - offset of the stick by gyroscope slopes (LB + RB + BACK)
2. Steering wheel - complete emulation of the left stick by the gyroscope (LB + RB + START)
3. By default - no gyroscope using (LB + RB + BACK + START)

To center the axes, press - LB + RB + DPAD UP.

## Setup
Change the COM port number in the file "X360Advance.ini", the sensitivity of the steering wheel and the rotation of the camera if necessary.

Next you need to copy the files "xinput1_3.dll" (for 32 bit games and for 64 bit copy "xinput1_3x64.dll" and rename to "xinput1_3.dll") and "X360Advance" in the game folder and run the game.

Perhaps for some games you will have to rename "xinput1_3.dll" to one of the names: "xinput1_4.dll" (Windows 8 / metro apps only), "xinput1_2.dll", "xinput1_1.dll" or "xinput9_1_0.dll

## Arduino
Need some kind of rotation sensor for example the MPU 6050.

An example of firmware for X360Advance can be found [here](https://github.com/r57zone/X360Advance-Arduino/blob/master/Arduino/Arduino.Output.ino).

## Download
>Version for Windows XP, 7, 8.1, 10.

**[Download](https://github.com/XInputNext/X360Advance/releases)**

## Feedback
`r57zone[at]gmail.com`
