[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/r57zone/X360Advance/) 
[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/r57zone/X360Advance/blob/master/README.RU.md)
← Choose language | Выберите язык

# X360Advance 
External Arduino gyroscope for any XInput compatible gamepads. With it, you can more accurately aim and steer. Buttons allow you to switch modes directly during the game. [External pedals are supported](https://github.com/r57zone/XboxExternalPedals), you can use them even without a gyroscope.

[![YouTube-X360Advance](https://user-images.githubusercontent.com/9499881/52436336-77815c80-2b2d-11e9-8d56-4ff82d82f48c.gif)](https://youtu.be/lNH2shRDchw)
[![YouTube-X360Advance](https://user-images.githubusercontent.com/9499881/52436371-91bb3a80-2b2d-11e9-8bd1-3399e4026962.gif)](https://youtu.be/lNH2shRDchw)


There are 3 modes of use:

1. Default - no use of a gyroscope (button 1 - digital pin 5)
2. Steering wheel - emulation of the left stick with a gyroscope (button 2 - digital pin 4)
3. FPS - mouse movement by tilting the gyroscope (button 3 - digital pin 3)

The centering of the axes occurs when you press the steering wheel button or FPS.

## Setup: first method (recommended)
The [first way](https://youtu.be/jzjp3BKtdSs) is to use the "XInput Injector" program. It is located in the notification area and allows you to infiltrate the game process in a couple of clicks, and use a gyroscope.

![](https://user-images.githubusercontent.com/9499881/69274645-3a85e280-0bf4-11ea-9df6-31a8e2b8dc62.png)


Before use, you just need to run the "X360Advance Settings" program, enter the COM port number, change the sensitivity of mouse and steering wheel movement, the default settings are recommended. Further, the sensitivity of the mouse can be changed already in the games themselves.

For convenience, you can also add "XInput Injector" to startup.

In the "XInput Injector" program itself, it is enough to select the name of the process and click on it; after successful implementation, the sound of connecting new equipment will be reproduced.

## Setup: second method
The second way to use it is to copy the proxy dll to the folder of the game itself. The method has poor compatibility, works mainly with old games.

To configure, you need to change the COM port number in the "X360Advance.ini" file, the sensitivity of mouse and steering wheel movement, the default settings are recommended.

Next, you need to copy the files "xinput1_3.dll" (according to the bitness of the game, for a 32-bit game from the x86 folder, for a 64-bit game from the 64 folder) in the folder with the game and start the game.

It may be necessary for some games to rename "xinput1_3.dll" to one of the names: "xinput9_1_0.dll", "xinput1_1.dll", "xinput1_2.dll" or "xinput1_4.dll" (only Metro applications).

For games in which the XInput dll library does not work, you can also use the "X360AdvanceApp" application, which allows you to simply emulate a mouse for aiming.

## External Arduino Gyroscope
Need to buy [Arduino Nano](http://ali.pub/2oy73f), rotation sensor [MPU 6050 GY-521](http://ali.pub/2oy76c), [3 buttons](http://ali.pub/33lzue), [prototype board](http://ali.pub/340eo5), [cable 2m miniUSB](http://ali.pub/340epp) or [microUSB](http://ali.pub/340eqa) (depending on the arduino) and [tape Velcro](http://ali.pub/33pbqa). Solder the board by scheme. Attach to the back of the case, for example, with rubber bands.

![](https://user-images.githubusercontent.com/9499881/52437030-42760980-2b2f-11e9-8ce5-14b45b30ca31.png)

![](https://user-images.githubusercontent.com/9499881/60760041-9aae2a80-a03f-11e9-81a0-e87cf84a0660.png)


Also can make just one wire using a USB hub. The [USB hub](http://alii.pub/6d2ngp) has a small board and can be placed inside the gamepad.

![](https://user-images.githubusercontent.com/9499881/60759864-dc89a180-a03c-11e9-8bf4-d0b84894c0e1.png)


Also can pack everything in the controller case. To do this, remove the board support elements from the back of the inside.

[![](https://user-images.githubusercontent.com/9499881/85574543-ed656580-b647-11ea-9a93-e8802a5c209b.png)](https://user-images.githubusercontent.com/9499881/85574534-ec343880-b647-11ea-9107-df005d7aa7b9.png)


Flash calibration sketch, put on a flat surface, get the data for calibration. [Insert calibration data into the main sketch](https://youtu.be/sKuiGC6Mxf0?t=184).

Firmware and libraries can be found at the link below.

## Possible problems
Some games are incompatible with this method, since they are not designed to use a mouse and a gamepad at the same time. Some games can constantly change button icons (from keyboard to gamepad and vice versa).

## Credits
* [MinHook](https://github.com/TsudaKageyu/minhook) for the Windows hook library.
* [Injector by @Nefarius](https://github.com/nefarius/Injector) for a handy DLL injector.
* [Injectable Generic Camera System by Frans Bouma](https://github.com/ghostinthecamera/IGCS-GITC) for examples of using MinHook to spoof XInput data.

## Download
>Version for Windows 7, 8.1, 10.

**[Download](https://github.com/r57zone/X360Advance/releases)**

## Feedback
`r57zone[at]gmail.com`
