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


