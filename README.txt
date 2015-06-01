arduino libraries and programs using TinyWireM library for I2C bus interface on
ATtiny processors

May 24, 2015 - Initial posting - G. D. (Joe) Young

To use the I2C bus with Atmel processors which do not have the TwoWire hardware
included on the 'standard' arduino processors (ATMegaxxxx) another arduino
library package, TwoWireM, was developed by BroHogan in 1/21/2011 (see 
TinyWireM.h, for details). Also, a modification of the LiquidCrystal_I2C libary
enabled using that library with ATtiny processors.

These TinyWireM developments are being incorporated into some of the other
libraries in this repository. In particular, the Keypad_I2C has a TinyWireM
version, and the DS3231 clock library switches to TinyWire when compiled with
an ATtiny board selection.


May 31/15

The library LCDBarCentreZero_I2C is tested with the TinyWireM liquid crystal 
library and works without modification - the example sketch fourBarsI2Ctw 
required changing only the #include from <Wire.h> to <TinyWireM.h>

The DS3231 clock library and Keypad_tw library were headed for an ATtiny85 
clock which could be set with a keypad. However, the combined libraries with the
clock display and keypad setting functions occupy about 9.9K of memory on the
standard arduino. I put it into the ATtiny anyway, and it just fit--using
8112 bytes. Still, the program did not work. Some investigation to find out
the size of used read-write memory indicates that the program clocDispDS_kpd
(in standard arduino) is using at least 688 bytes of rwm. Similar separate
tests of clockDispDStw and getKeyLcdtw find rwm use 242 and 282, respectively.
So even though the program code for the combined clock, display, and keypad
fits, there is not enough rwm in the ATtiny85 to run.

The keypad--display combination for data entry is illustrated with example 
getKeyLcd. Its size is about 5K in the tinywire version.

