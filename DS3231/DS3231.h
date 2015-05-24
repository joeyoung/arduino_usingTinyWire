// DS3231.h - header for DS3231 version
// I2C clock access package

// started: Oct 22, 2013 G. D. (Joe) Young <jyoung@islandnet.com>

// revised: Oct 28/13 - NRREG moved to constructor
//			Nov 2/13 - modified so storage is with user
//          Mar 04/14 - gets return error, remove getErr
//			Mar 17/14 - getStatus, setStatus isolated block versions
//          Apr 30/15 - wrappers for using TinyWire on ATtiny85 for I2C bus access

// Implementation for DS3231, I2C, temp compensated, clock IC

#ifndef DS3231_HDR
#define DS3231_HDR

#include "clock.h"
//#include "Wire.h"
#if defined(__AVR_ATtiny85__) || (__AVR_ATtiny2313__)
#include "TinyWireM.h"      // include this if ATtiny85 or ATtiny2313
#else 
#include <Wire.h>           // original lib include
#endif

// datasheet bit name definitions
#define CN 7	//century, in date.mo byte
// alarm enables
#define A1M1 7
#define A1M2 7
#define A1M3 7
#define A1M4 7
#define A2M2 7
#define A2M3 7
#define A2M4 7
#define DY_DT 6
// control/status
#define EOSC 7
#define BBSQW 6
#define CONV 5
#define RS2 4
#define RS1 3
#define INTCN 2
#define A2IE 1
#define A1IE 0
#define OSF 7
#define EN32KHZ 3
#define BSY 2
#define A2F 1
#define A1F 0

#define ALARM1SIZE 4
#define ALARM2SIZE 3
#define DSSTATUS 0x0E
#define DSSTATMP 0x11

class DS3231 : public clock {

public:

	DS3231( );

	byte getTime( Time & );
	byte getDate( Date & );
	byte getStatus( byte, byte, byte * );	//array: adr, count, array destination
	byte getStatus( byte, byte * );			//single-byte: adr, destination for status byte
	byte getAlarm( byte almNr, byte * );
	byte setTime( Time &setT );
	byte setDate( Date &setD );
	byte setStatus( byte, byte, byte *setS );	//array: adr, count, array source
	byte setStatus( byte, byte );				//single-byte, adr, value
	byte setAlarm( byte almNr, byte *setA );

//	byte getErr( );				// enable for debugging; not otherwise needed


private:
	static const byte NRREG = 19;		//number of clock chip's registers
	static const byte I2Cadr = 0x68;	//7-bit I2C address
	byte errCode;

//	byte dsRegs[NRREG];		// array to hold copy of clock internal values (simulate I2C access)

//	byte writeClock( byte regadr, byte count, byte *regs );
	void writeClock( byte regadr );
    void i2write( byte val );
    byte i2end( );
//	byte readClock( byte regadr, byte count, byte *regs );
	byte readClock( byte regadr, byte count );
    byte i2read( );
    void i2start( byte adr );
	static const byte dsDate = 0x03;	//offset to start of Date register block
//	static const byte dsStatus = 0x0E;	//offset to start of Status registers
	static const byte dsAlarm = 0x07;	//offset to start of alarm registers
	static const byte dsTime = 0x00;	//offset to start of time registers
//	static const byte dsTemp = 0x11;	//offset to start of temp in status

}; // class DS3231

#endif


