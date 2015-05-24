// I2C clock access package

// started: Oct 22, 2013 G. D. (Joe) Young <jyoung@islandnet.com>

// revised: Feb 23/14 - structs for time and date
//          Mar 04/14 - write err constants, gets return error.
//          Mar 17/14 - getStatus, setStatus for individual blocks, single byte

// Present a common set of access functions to work with different clock IC:
//
// getTime, getDate, getStatus, to read IC registers; corresponding set
// functions for setting. Implement as a base class with these functions as
// virtual functions, with function implementation in unique-to-IC class so
// that register addressing, arrangement, etc. are hidden from users of the
// access functions. Works fairly well for Date and Time; but alarms and 
// status will be fairly IC-specific

#ifndef CLOCK_HDR
#define CLOCK_HDR

#include "Arduino.h"

#define RTC_OK 0
#define RTC_RD_ERR 1
#define RTC_TM_RD_ERR 2
#define RTC_DT_RD_ERR 3
#define RTC_ST_RD_ERR 4
#define RTC_AL_RD_ERR 5
#define RTC_SET_ERR 6
#define RTC_WR_ERR 7
#define RTC_TM_WR_ERR 8
#define RTC_DT_WR_ERR 9
#define RTC_ST_WR_ERR 10
#define RTC_AL_WR_ERR 11

struct Time {
	byte sec;
	byte min;
	byte hr;
};

struct Date {
	byte dow;
	byte dom;
	byte mo;
	byte yr;
};

class clock {

public:
	virtual byte getTime( Time & ) = 0;
	virtual byte getDate( Date & ) = 0;
	virtual byte getStatus( byte, byte, byte * ) = 0;	// clk reg adr, count, return array
	virtual byte getStatus( byte, byte * ) = 0;			// clk reg adr, single value
	virtual byte getAlarm( byte almNr, byte * ) = 0;


	virtual byte setTime( Time &setT ) = 0;
	virtual byte setDate( Date &setD ) = 0;
	virtual byte setStatus( byte, byte, byte *setS ) = 0;	// clk reg adr, count, array to set
	virtual byte setStatus( byte, byte ) = 0;				// clk reg adr, value
	virtual byte setAlarm( byte almNr, byte *setA ) = 0;


private:

}; // class clock

#endif

