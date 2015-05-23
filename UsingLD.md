## Client software ##

To get something useful you'll need logic analyzer client running on PC.
Theoretically any SUMP compatible app will do. I'm using [Logic Sniffer](http://www.lxtreme.nl/ols/)

## Board ##

By default LD uses port D as it's data source. Triggers can be configured by client software to use any edge on any pin, sampling can also be triggered by pressing user button (blue one).

PA2 is a test output. It's uart with the following settings: 115200bps, 1 stop bit and no parity. Contents of transmitted message may vary from version to version (usually it's a simple 1 byte counter), but settings will be kept.

## Connecting LD to PC ##

Since [r8](https://code.google.com/p/logicdiscovery/source/detail?r=8) LogicDiscovery supports two interfaces: **USB** or **UART**. Only one can be active at a time.

Interface is selected using **PC1** pin. It has pull-up enabled, and defaults to using USB. Connect **PC1** to **GND** to enable UART.
  * UART pins: **PC7(RX)** and **PC6(TX)**.
  * UART settings: 115200bps, 1 stop bit, no parity.

STM32F4Discovery board is powered from the mini-USB port (STLing). To power board from the micro-USB and get rid of a second cable connect PA9 pin to 5V pin on P2 connector. PA9 is wired directly to the the micro-USB's VBus, and 5V pin is wired to the LDO's input.