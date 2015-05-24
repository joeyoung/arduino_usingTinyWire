// DS3231.cpp -  DS3231 version
// I2C clock access package

// started: Oct 22, 2013 G. D. (Joe) Young <jyoung@islandnet.com>

// revised: Nov 2/13 - rearrange so base has storage
//			Feb 25/14 - Arduino version started
//          Mar  4/14 - gets return errCode, remove itermediate storage
//			Mar 17/14 - revised getStatus, setStatus
//          Apr 30/15 - wrappers for using TinyWire on ATtiny85 for I2C bus access
//          May 20/15 - fix readClock error return for TinyWire

// The TinyWireM.requestFrom call does not return the actual number of bytes read
// as the Wire.requestFrom does. Consequently, the error check following readClock 
// in the getXXX functions did not work. The wrapper in readClock was modified to
// emulate the Wire.requestFrom when using the TinyWireM library. 

// Implementation for DS3231, I2C, temp compensated, clock IC

#include "DS3231.h"

DS3231::DS3231( ) {
	errCode = 0;
} // constructor


byte DS3231::getTime( Time &dst ) {
	byte err = readClock(  dsTime, 3 );	// request time
	if( err != 3 ) {
		errCode = RTC_TM_RD_ERR;
		return errCode;
	} else {
		dst.sec = i2read( );
		dst.min = i2read( );
		dst.hr = i2read( );
		return RTC_OK;
	} // if err
} // getTime


byte DS3231::getDate( Date &dsd ) {
	byte err = readClock(  dsDate, 4 );	// request date
	if( err != 4 ) {
		errCode = RTC_DT_RD_ERR;
		return errCode;
	} else {
		dsd.dow = i2read( );
		dsd.dom = i2read( );
		dsd.mo = i2read( );
		dsd.yr = i2read( );
		return RTC_OK;
	} // if err
} // getDate

byte DS3231::getStatus( byte regadr, byte cnt, byte *ckS ) {
	byte err = readClock( regadr, cnt );	// request cnt status bytes
	if( err != cnt ) {
		errCode = RTC_ST_RD_ERR;
		return errCode;
	} else {
		for( byte ix = 0; ix < cnt; ix++ ) *(ckS+ix) = i2read( );
	}
	return RTC_OK;
} // getStatus - array


byte DS3231::getStatus( byte regadr, byte *ckS ) {
	byte err = readClock( regadr, 1 );		//request single byte
	if( err != 1 ) {
		errCode = RTC_ST_RD_ERR;
		return errCode;
	} else {
		*ckS = i2read( );
	}
	return RTC_OK;
} // getStatus - single byte


byte DS3231::getAlarm( byte almNr, byte *ckA ) {
	byte nrbytes = 3;
	if( almNr == 0 ) nrbytes = 4;
	byte err = readClock( dsAlarm+almNr*4, nrbytes );
	if( err != nrbytes ) {
		errCode = RTC_AL_RD_ERR;
		return errCode;
	} else {
		for( byte ix = 0; ix < nrbytes; ix++ ) *(ckA+ix) = i2read( );
		return RTC_OK;
	}
} // getAlarm


byte DS3231::setTime( Time &setT ) {
	writeClock( dsTime );	// setup writing to time registers
	i2write( setT.sec );
	i2write( setT.min );
	i2write( setT.hr );
	errCode = i2end( );
	if( errCode == 0 ) {
		return RTC_OK;
	} else {
		return RTC_TM_WR_ERR;
	}
} // setTime


byte DS3231::setDate( Date &setD ) {
	writeClock( dsDate );	// setup writing to date registers
	i2write( setD.dow );
	i2write( setD.dom );
	i2write( setD.mo );
	i2write( setD.yr );
	errCode = i2end( );
	if( errCode == 0 ) {
		return RTC_OK;
	} else {
		return RTC_DT_WR_ERR;
	}
} // setDate


byte DS3231::setStatus( byte regadr, byte cnt, byte *setS ) {
	writeClock( regadr );
	for( byte ix = 0; ix < cnt; ix++ ) i2write( setS[ix] );
	errCode = i2end( );
	if( errCode == 0 ) {
		return RTC_OK;
	} else {
		return RTC_ST_WR_ERR;
	}
} // setStatus - array of bytes


byte DS3231::setStatus( byte regadr, byte val ) {
	writeClock( regadr );
	i2write( val );
	errCode = i2end( );
	if( errCode != 0 ) return RTC_ST_WR_ERR;
	return RTC_OK;
} // setStatus - single byte


byte DS3231::setAlarm( byte almNr, byte *setA ) {
	writeClock( dsAlarm+4*almNr );
	if( almNr == 0 ) for( byte ix = 0; ix < 4; ix++ ) i2write( setA[ix] );
	if( almNr == 1 ) for( byte ix = 0; ix < 3; ix++ ) i2write( setA[ix] );
	errCode = i2end( );
	if( errCode == 0 ) {
		return RTC_OK;
	} else {
		return RTC_AL_WR_ERR;
	}
} // setAlarm

void DS3231::writeClock( byte regadr ) {
	i2start( I2Cadr );	// start write with addr
	i2write( regadr );				// then start of block register adr
} // I2C write block - beginning of Wire operation

void DS3231::i2write( byte val ) {
  #if defined(__AVR_ATtiny85__) || (__AVR_ATtiny2313__)   // Replaced Wire calls with ATtiny TWI calls
  TinyWireM.send( val );
  #else
  Wire.write( val );
  #endif
} // i2c byte write operation with wire library switch

byte DS3231::i2end( ) {
  #if defined(__AVR_ATtiny85__) || (__AVR_ATtiny2313__)   // Replaced Wire calls with ATtiny TWI calls
  return( TinyWireM.endTransmission( ) );
  #else
  return( Wire.endTransmission( ) );
  #endif
} // endTransmission wrapper to switch between libs

void DS3231::i2start( byte adr ) {
  #if defined(__AVR_ATtiny85__) || (__AVR_ATtiny2313__)   // Replaced Wire calls with ATtiny TWI calls
  TinyWireM.beginTransmission( adr );
  #else
  Wire.beginTransmission( adr );
  #endif
} // beginTransmission wrapper to switch between libs


byte DS3231::readClock( byte regadr, byte count ) {
  i2start( I2Cadr ); 	// start write to slave
  i2write( regadr );     // set chip's adr pointer for this read
  i2end();
  #if defined(__AVR_ATtiny85__) || (__AVR_ATtiny2313__)   // Replaced Wire calls with ATtiny TWI calls
  return( TinyWireM.requestFrom( I2Cadr, count ) + count );		// if error retn == 0 this returns count
  #else
  return( Wire.requestFrom( I2Cadr, count ) );
  #endif
} // I2C read block

byte DS3231::i2read( ) {
  #if defined(__AVR_ATtiny85__) || (__AVR_ATtiny2313__)   // Replaced Wire calls with ATtiny TWI calls
  return( TinyWireM.receive( ) );
  #else
  return( Wire.read( ) );
  #endif
} // i2c byte read from buffer

// debug access for errCode
//byte DS3231::getErr( ) {
//	return errCode;
//} 

