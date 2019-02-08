[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/r57zone/X360Advance-Arduino/blob/master/README.md) 
[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/r57zone/X360Advance-Arduino/blob/master/README.RU.md) 
# X360Advance (XInput) 
Внешний Arduino гироскоп для Xbox геймпада, который позволяет использовать его как руль, а также помогать более точно прицеливаться.

![](https://user-images.githubusercontent.com/9499881/52436336-77815c80-2b2d-11e9-8d56-4ff82d82f48c.gif)
![](https://user-images.githubusercontent.com/9499881/52436371-91bb3a80-2b2d-11e9-8bd1-3399e4026962.gif)


Есть 3 режима использования:

1. По умолчанию - отсуствие использования гироскопа (кнопка 1 - digital pin 5)
2. Руль - эмуляция левого стика гироскопом (кнопка 2 - digital pin 4)
3. FPS - движение мышкой наклонами гироскопа (кнопка 3 - digital pin 3)

Центрирование осей происходит при нажатии кнопки руля или FPS.


При возникновении проблем, например, беспорядочном вращении, можно попробовать сбросить буфер ком порта Arduino, для этого нужно нажать кнопки: левый бампер, правый бампер, back и start.

## Настройка
Измените номер COM-порта в файле "X360Advance.ini", чувствительность руля и вращения камеры, если нужно.

Далее необходимо скопировать файлы "xinput1_3.dll" (для 32 битных игр, а для 64 битных скопировать "xinput1_3x64.dll" и переименовать в "xinput1_3.dll") в папку с игрой и запустить игру. 

Возможно, для некоторых игр придется переименовать "xinput1_3.dll" в одно из названий: "xinput1_4.dll" (Windows 8 / metro apps only), "xinput1_2.dll", "xinput1_1.dll" или "xinput9_1_0.dll".

## Arduino
Необходимо купить [Arduino Nano](http://ali.pub/2oy73f), датчик вращения [MPU 6050 GY-521](http://ali.pub/2oy76c), [3 кнопки](http://ali.pub/33lzue), [ленту липучку](http://ali.pub/33pbqa) и кабель 2 м. miniUSB или microUSB (в зависимости от Arduino). Спаять по схеме. Закрепить на задней части корпуса, например, резинками.

![](https://user-images.githubusercontent.com/9499881/52437030-42760980-2b2f-11e9-8ce5-14b45b30ca31.png)

![](https://user-images.githubusercontent.com/9499881/52437903-78b48880-2b31-11e9-81ac-7b639286db70.png)


Прошить скетч калибровки, положить на ровную поверхность, получить данные для калибровки. Вписать данные калибровки в основной скетч.

Прошивку и библиотеки можно найти по ссылке ниже.

## Загрузка
>Версия для Windows XP, 7, 8.1, 10.

**[Загрузить](https://github.com/r57zone/X360Advance-Arduino/releases)**

## Обратная связь
`r57zone[собака]gmail.com`
