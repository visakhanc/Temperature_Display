About
-----

This is a simple digital thermometer based on LM35 temperature sensor. It uses AVR ATmega8 microcontroller to sample LM35 sensor and display the temperature in degree celcius to a 3 digit segment display. 


Hardware
--------

AVR ATmega8 (uses 1MHz internal clock)
LM35 Temperature sensor
3 digit 7-Segment display (common anode)
Capacitors - 0.1uF x 3
Resistors - 10k x 1; 100R x 3
Power supply: 5V


Operation
---------

Samples are taken at every 30ms and accumulated. Average of 16 samples is calculated for displaying the temperature. Segment LCD is driven from AVR ports directly, since less than 10 mA is used by each segment, by 100 ohm current limiting resistors. A common anode 3 digit multiplexed 7 segment LCD is used. Each digit is driven for 2 ms by using Timer0 interrupt.


How to build the project
------------------------

AVR-GCC is used to compile the project. On Windows, WinAVR is the easy way to install AVR-GCC.
Extract the source code to a folder. On the command line go into the folder and type 'make all'
The output hex file need to be programmed to ATmega8. For this, I use avrdude utility and usbasp programmer. In this case, just type 'make program' after connecting the programmer. If different programmer is used modify the Makefile 'AVRDUDE' section. 
1MHz internal clock is used, so the fuses need to be set accordingly: HFUSE = 0xd9, LFUSE = 0xe1


For any queries, feel free to contact me:
visakhanc@gmail.com

