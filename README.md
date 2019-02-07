[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/r57zone/X360Advance-Arduino/blob/master/README.md) 
[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/r57zone/X360Advance-Arduino/blob/master/README.RU.md)
# X360Advance (XInput) 
External Arduino Gyroscope for the Xbox gamepad that allows to use it as a steering wheel, and also help more accurately aim. There is also a [version for OpenTrack](https://github.com/r57zone/X360Advance).

![](https://user-images.githubusercontent.com/9499881/52436336-77815c80-2b2d-11e9-8d56-4ff82d82f48c.gif)
![](https://user-images.githubusercontent.com/9499881/52436371-91bb3a80-2b2d-11e9-8bd1-3399e4026962.gif)


There are 3 modes of use:

1. Default - no use of the gyroscope (button 1 - digital pin 5)
2. Steering wheel - emulation of the left stick by the gyro (button 2 - digital pin 4)
3. FPS - mouse movement by tilting the gyro (button 3 - digital pin 3)

The centering of the axes occurs when you press the steering wheel button or FPS.

## Setup
Change the COM port number in the file "X360Advance.ini", the sensitivity of the steering wheel and the rotation of the camera if necessary.

Next you need to copy the files "xinput1_3.dll" (for 32 bit games and for 64 bit copy "xinput1_3x64.dll" and rename to "xinput1_3.dll") and "X360Advance" in the game folder and run the game.

Perhaps for some games you will have to rename "xinput1_3.dll" to one of the names: "xinput1_4.dll" (Windows 8 / metro apps only), "xinput1_2.dll", "xinput1_1.dll" or "xinput9_1_0.dll

## Arduino
Need to buy [Arduino Nano](http://ali.pub/2oy73f), rotation sensor [MPU 6050 GY-521](http://ali.pub/2oy76c) and [3 buttons](http://ali.pub/33lzue). Solder the scheme. Attach to the back of the case, for example, with rubber bands.

![](https://user-images.githubusercontent.com/9499881/52437030-42760980-2b2f-11e9-8ce5-14b45b30ca31.png)


Flash calibration sketch, put on a flat surface, get the data for calibration. Insert calibration data into the main sketch.

Firmware and libraries can be found at the link below.

## Download
>Version for Windows XP, 7, 8.1, 10.

**[Download](https://github.com/r57zone/X360Advance-Arduino/releases)**

## Feedback
`r57zone[at]gmail.com`
