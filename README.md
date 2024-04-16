Copyright (C)2023, VadRov, all right reserved / www.youtube.com/@VadRov / www.dzen.ru/vadrov
# LED-контроллер для отображения графической информации на экране, состоящем из соединенных последовательно лент с адресными светодиодами на чипе ws2812b. 
LED-контроллер отображает на экране информацию из AVI-файла (файл с расширением avi, контейнер riff), кадры которого представляют собой изображения в формате несжатого bmp (данные пикселей в файле хранятся как двумерный массив).\
LED-контроллер собран на монтажной плате с использованием отладочного мудуля на базе микроконтроллера STM32G031F6P6/STM32G031F8P6. К микроконтроллеру подключен дисплей ST7789 (через spi), модуль (адаптер/переходник на micro-SD) SD-карты (через spi), инкрементарный механический энкодер с кнопкой. В контроллере реализован файловый менеджер с графическим интерфейсом для просмотра содержимого sd-карты и выбора файлов для воспроизведения на LED-экране. Контроллер имеет 1 выход (порт) для управления адресными светодиодными лентами. Протокол управления адресными светодиодными лентами реализован посредством PWM c DMA с таймингами управляющего сигнала, соответствующими спецификации модулей ws2812b. Для корректной передачи сигналов на первый адресный чип, выходной порт контроллера реализован по схеме открытый сток и подтянут к питанию +5В (резистор R2 на схеме), что обеспечивает уровни сигналов на входе DI, соответствующими спецификации ws2812b. Резистор R1 ограничивает ток, проходящий через выходной вывод микроконтроллера и предохраняет его от повреждения в том случае, например, если пользователь захочет обновить ПО микроконтроллера, забыв при этом отключить его от экрана с отключенным питанием (в этом случае лента будет "тянуть питание" через этот вывод, и он может повредиться при превышения допустимого значения тока).
Перемещение по файлам и каталогам в файловом менеджере осуществляется путем вращения ручки энкодера. Выбор avi-файла для воспроизведения или переход в подкаталог(возврат в каталог) осуществляется коротким нажатием (кликом) кнопки энкодера. Для остановки воспроизведения текущего файла следует длительно (2 и более секунд) удерживать нажатой, а затем отпустить кнопку энкодера. 
# Подготовка файлов для воспроизведения
Как было отмечено выше, LED-контроллер воспроизводит AVI-файлы (файл с расширением avi, контейнер riff). Формат файла должен соответствовать спецификации microsoft. При этом кадры видеопотока AVI-файла должны быть в формате несжатого BMP (несжатые данные RGB, хранящиеся последовательно, как двумерный массив). Для генерации последовательности BMP-изображений (цветовые эффекты, бегущие строки и т.д.), формирующих непрервывный видеоряд, можно, например, воспользоваться программой Jinx!. Настройка Jinx! заключается в определении опций матрицы (ширина и высота матрицы в пикселях - в нашем случае выражается в количестве светодиодных модулей на экране по горизонтали и вертикали), а также свойств выводного устройства (куда и как сохранять изображения). Размеры матрицы задаются через Setup->Matrix Options. 
![jinx_matrix_options](https://github.com/vadrov/stm32g031f6p6-led-lent-screen-controller/assets/111627147/c513b7a0-72bb-46d3-817d-3182883e8733)

Параметры устройства вывода определяются через Setup->Output Devices, где следует выбрать Add. Затем в списке Devices Type выбирается пункт Bitmap Export. Требуемый каталог для хранения последовательности изображений задают через кнопку Select. Выбрав и настроив требуемый эффект, можно стартовать запись кадров в указанный каталог. Для этого необходимо выбрать в меню Setup->Start Output. По просшествии требуемого периода времени, запись можно остановить, снова выбрав в меню Setup->Start Output.
![jinx_output_devices](https://github.com/vadrov/stm32g031f6p6-led-lent-screen-controller/assets/111627147/a01735f5-6dc6-463a-a37c-f295db8f879b)

Полученные кадры изображения можно "склеить" в один AVI-файл. Для этого можно использовать библиотеку ffmpeg, введя, например, такую командную строку, находясь в каталоге с картинками:
```
ffmpeg -i %8d.bmp -r 25 -vcodec bmp video.avi
где:
-i %8d.bmp - входные данные (файлы-картинки). %8d означает, что имя файла состоит из 8-значного числа;
-r 25 - частота кадров (25 кадров/секунду) в получаемом видеофайле (этот параметр указан в окне Jinx!);
-vcodec bmp - используется кодек bmp - указание на то, что используются несжатые данные RGB;
video.avi - название выходного (целевого) файла.
```
Другой способ получения требуемого формата изображения для LED-экрана заключается в конвертации любого видеофайла той же библиотекой ffmpeg. Например:
```
ffmpeg -i file.mp4 -r 25 -vcodec bmp -s 60x20 video.avi
где:
-i file.mp4 - входной видеофайл;
-r 25 - частота кадров (25 кадров/секунду) - Вы можете, например, ограничить частоту кадров;
-vcodec bmp - используется кодек bmp - указание на то, что используются несжатые данные RGB;
-s 60x20 - размер кадра 60 на 20 пикселей (один пиксель соответствует одному светодиодному модулю);
video.avi - название выходного (целевого) файла.
```
Дополнительной опцией -an можно исключать звуковую дорожку из файла.\
Допускается свободное распространение. При любом способе распространения указание автора ОБЯЗАТЕЛЬНО. В случае внесения изменений и распространения модификаций указание первоначального автора ОБЯЗАТЕЛЬНО. Распространяется по типу "как есть", то есть использование осуществляется на свой страх и риск. Автор не предоставляет никаких гарантий.
## Компоненты:
- Отладочная плата (МК) STM32G031F6P6/STM32G031F8P6;
- SPI дисплей на контроллере ST7789 разрешением 240x240;
- адаптер/переходник с SD карты на микро-SD;
- инкрементарный механический энкодер;
- 2 резистора с номиналами: 390-680 Ом и 4.7-8.2 кОм;
- адресные светодиодные ленты для формирования экрана (в данном проекте используется 20 лент по 60 адресных светодиодов на чипе ws2812b. Итого 1200 адресных светодиода для экрана с разрешением 60х20);
- блок питания на 5В для запитывания контроллера и экрана из расчета 0.3 Вт на один модуль ws2812b (в данном проетке используется блок питания на 350 Вт. При свечении всех модулей белым цветом просадок по напряжению нет).
 
https://github.com/vadrov/stm32g031f6p6-led-lent-screen-controller/assets/111627147/38e19da3-de05-4f13-ab73-b497f85633d7

https://github.com/vadrov/stm32g031f6p6-led-lent-screen-controller/assets/111627147/aa63d80d-37d3-4e82-a94f-37d700e6b066

![схема соединений LED контроллера](https://github.com/vadrov/stm32g031f6p6-led-lent-screen-controller/assets/111627147/7e9db14c-1e46-4c8c-ba71-1c4ffcef843f)

![controller_side1](https://github.com/vadrov/stm32g031f6p6-led-lent-screen-controller/assets/111627147/383d8c5a-8d73-4b3e-b896-358a0a315c2e)

![controller_side2](https://github.com/vadrov/stm32g031f6p6-led-lent-screen-controller/assets/111627147/6ea11f9b-ff75-495c-a7b1-adb1f4f86806)

![screen](https://github.com/vadrov/stm32g031f6p6-led-lent-screen-controller/assets/111627147/9ab5b044-a5a9-4ee6-aa04-8c1c01cf9d0f)



