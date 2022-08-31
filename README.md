# w806-emu2413

This is the YM2413 emulator by Mitsutaka Okazaki and Rupert Carmichael (https://github.com/digital-sound-antiques/emu2413) ported to the WM806 Microcontroller.

The program opens up the Miditones song file in the memory and play it with the EMU2413.

Requirements:
- The W806 SDK from https://github.com/IOsetting/wm-sdk-w806
- I2S DAC (PCM5102)
- EMU2413 version 1.5.7

Wiring table to connect this to PCM5102 board (you can find these in AliExpress):

|W806|PCM5102|
|---|---
|PB8 (BCK)|BCK|
|PB9 (LRCK)|LCK|
|PB11 (DO)|DIN|

***Attention: Make sure the PCM5102 board's FMT pin is grounded! Else there will be no or wrong output!***

Limitations:
- Some of the EMU2413 code has been changed to fit into that embedded system. Means that there are no memory allocations and a number of the tables (especially the large ones like tll_table) are precalculated with constexpr.
- Limited resampling by averaging (USE_FAST_RATE_CONV) and not using sinc interpolation.
- Without the resamplers active, the output frequency is off compared to the actual emulation of YM2413 on other platforms. Currently it is not known how much output frequency errors are present.
- The percussions are not tested yet. It is not sure if the percussive sounds are being rendered correctly without that mentioned resampler.
- There are clicking noises between note switches - this is mitigated by using an older version of the Miditones (v1.12) where there are note stops before the note change happens. This note stops allow the brief release of the note in the envelope generator and significantly minimizes the unpleasent noise.
- The OPLL calculations inside require quite a number of computation power, the sampling is only up to 22050Hz.

Extra notes:
- The sine table is precalculated using constexpr with math functions, and this requires a C++14 and beyond. In the **wm-sdk-w806\tools\W806\conf.mk**, please change the config so that it is using "***gnu++14***" instead.
- Commenting out the CONFIG_KERNEL_NONE = 1 when using FreeRTOS is causing the other interrupts not to be triggered. A temporary fix is discussed in this [issue](https://github.com/IOsetting/wm-sdk-w806/issues/25).
