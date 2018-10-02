[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/XInputNext/X360Advance/blob/master/README.md) 
[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/XInputNext/X360Advance/blob/master/README.EN.md) 
# X360Advance (XInput) 
Внешний Arduino гироскоп для Xbox геймпада, который позволяет использовать его как руль, а также помогать более точно прицеливаться. [Также есть версия для OpenTrack](https://github.com/r57zone/X360Advance).

![](https://user-images.githubusercontent.com/9499881/27588504-749af800-5b59-11e7-92e4-2b3813428281.png)

Есть 3 режима использования:<br>
1. FPS - смещение стика наклонами гироскопа (LB + RB + BACK)
2. Руль - полная эмуляция левого стика гироскопом (LB + RB + START)
3. По умолчанию - отсуствие использования гироскопа (LB + RB + BACK + START)

Для центрирования осей нажмите - LB + RB + DPAD UP.

## Настройка
Измените номер COM-порта в файле "X360Advance.ini", чувствительность руля и вращения камеры, если нужно.

Далее необходимо скопировать файлы "xinput1_3.dll" (для 32 битных игр, а для 64 битных скопировать "xinput1_3x64.dll" и переименовать в "xinput1_3.dll") и "X360Advance" в папку с игрой, и запустить игру. 

Возможно, для некоторых игр придется переименовать "xinput1_3.dll" в одно из названий: "xinput1_4.dll" (Windows 8 / metro apps only), "xinput1_2.dll", "xinput1_1.dll" или "xinput9_1_0.dll".

## Arduino
Необходим какой-нибудь датчик вращения, например, MPU 6050.

Пример прошивки для X360Advance можно найти [здесь](https://github.com/r57zone/X360Advance-Arduino/blob/master/Arduino/Arduino.Output.ino).

## Загрузка
>Версия для Windows XP, 7, 8.1, 10.

**[Загрузить](https://github.com/XInputNext/X360Advance/releases)**

## Обратная связь
`r57zone[собака]gmail.com`
