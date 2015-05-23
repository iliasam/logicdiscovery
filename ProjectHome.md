Turns STM32F4Discovery board (or any stm32f4/stm32f2 board after some tuning) into SUMP-compatible logic analyzer.

## Features ##
  * Sampling rate up to 20MHz
  * 16 channels
  * up to 24k samples
  * Edge sensitive triggers
  * RLE encoding of samples for longer runs
  * USB and UART interfaces for connecting to host
  * SUMP-compatible protocol

## Limitations ##
Comapred to original SUMP device and it's clones
  * Maximum sampling rate is 20MHz instead of 200MHz
  * No noise filtering
  * RLE only up to 5MHz
  * Channel groups can be only first or first+second
  * Triggers are edge sensitive, matching by value is not supported

[Blog post on Habr](http://habrahabr.ru/post/165853/)