// setclock - serial clock setting

// created: Mar 2, 2014  G. D. (Joe) Young <jyoung@islandnet.com>

// revised: Mar 4/14 - use get's error return
//          Mar 12/14 - RV3029 version
//          Mar 17/14 - hex digit entry for alarms
//          May 20/15 - setup to use softwareSerial, to work with
//                      ATtiny85 and TinyWireM for I2C

// use clock libraries to access clock chips, collect time and date
// bytes from serial monitor and set clock chip.

//#include <Wire.h>
#include <TinyWireM.h>

#include <SoftwareSerial.h>
// Definitions
#define rxPin 3
#define txPin 4
SoftwareSerial ssSerial(rxPin, txPin);

#include "DS3231.h"      // use whichever library matches clock IC
//#include "PCF8563.h"
//#include "RV3029.h"


Time time;
Date date;
#ifdef RV3029_HDR
RV3029 clk;
byte alarm[ALARM_SIZE];
#endif
#ifdef DS3231_HDR
DS3231 clk;
byte alarm[ALARM1SIZE];  // use larger size for either
#endif
#ifdef PCF8563_HDR
PCF8563 clk;
byte alarm[ALARM_SIZE];
#endif

char commline[20];  // for input setting line of bytes
byte ccount = 0;  // count of input characters
byte errfl = 0;   // return from parsing functions

byte getLine( );


void setup( ) {
//  Wire.begin( );
  TinyWireM.begin( );
  ssSerial.begin( 9600 );
  commline[0] = 0;
  ssSerial.println( "Begin clock setting." );
  ssSerial.println( "Enter empty line (\\ only) to keep current values." );
  ssSerial.println( "Enter exact number of digit pairs with any separator," );
  ssSerial.println( " ending with \\, to change the displayed setting." );
  ssSerial.println( "" );
} // setup


void loop( ) {
  ssSerial.print( "date " );
  byte err = clk.getDate( date );
//  byte err = clk.getErr( );
//  if( err != 0 ) {
//    ssSerial.print( "read err: " );
//    ssSerial.print( err, HEX );
//    ssSerial.print( "   " );
//  }
  ssSerial.print( date.yr, HEX );
  ssSerial.print( '/' );
  ssSerial.print( date.mo, HEX );
  ssSerial.print( '/' );
  ssSerial.print( date.dom, HEX );
  ssSerial.print( ' ' );
  ssSerial.println( date.dow, HEX );

  ccount = getLine( );	// get line of input for date
  if( ccount == 12 ) {
    date.yr = ( ( (commline[0]&0xf)<<4) | (commline[1]&0xf) );
    date.mo = ( ( (commline[3]&0xf)<<4) | (commline[4]&0xf) );
    date.dom = ( ( (commline[6]&0xf)<<4) | (commline[7]&0xf) );
    date.dow = ( ( (commline[9]&0xf)<<4) | (commline[10]&0xf) );
    clk.setDate( date );
  } // if exactly 12 characters - yy mm dd dw\

  ssSerial.print( "time " );
  err = clk.getTime( time );
//  byte err = clk.getErr( );
  ssSerial.print( time.hr, HEX );
  ssSerial.print( ':' );
  ssSerial.print( time.min, HEX );
  ssSerial.print( ':' );
  ssSerial.println( time.sec, HEX );

  ccount = getLine( );	// get line of input for time
  if( ccount == 9 ) {
    time.hr = ( ( (commline[0]&0xf)<<4) | (commline[1]&0xf) );
    time.min = ( ( (commline[3]&0xf)<<4) | (commline[4]&0xf) );
    time.sec = ( ( (commline[6]&0xf)<<4) | (commline[7]&0xf) );
    clk.setTime( time );
  } // if exactly 9 characters - hh mm ss\
  
  ssSerial.print( "alarm " );
  err = clk.getAlarm( 0, alarm );
#ifdef DS3231_HDR
  for( byte ix=0; ix<ALARM1SIZE; ix++ ) {
#else
  for( byte ix=0; ix<ALARM_SIZE; ix++ ) {
#endif
      ssSerial.print( alarm[ix], HEX );
      ssSerial.print( " " );
  }
  ssSerial.println( "" );
  
  ccount = getLine( );  // get line of input for alarm
#ifdef DS3231_HDR
  if( ccount == 3*ALARM1SIZE ) {
    for( byte ix=0; ix<ALARM1SIZE; ix++ ) {
#else
  if( ccount == 3*ALARM_SIZE ) {
    for( byte ix=0; ix<ALARM_SIZE; ix++ ) {
#endif
      byte lix = 3*ix;
//      alarm[ix] = ( (commline[lix]&0x0f)<<4) | (commline[lix+1]&0x0f);
      if( commline[lix] < 'A' ) {
        alarm[ix] = ( (commline[lix]&0x0f)<<4);
      } else {
        alarm[ix] = ( (commline[lix]&0x0f)+9)<<4;
      }
      if( commline[lix+1] < 'A' ) {
        alarm[ix] |= (commline[lix+1]&0x0f);
      } else {
        alarm[ix] |= ((commline[lix]&0x0f)+9);
      }
    } // for all alarm bytes
    clk.setAlarm( 0, alarm );
  } // if exactly right size for this alarm


} // loop


byte getLine( ) {
  byte lcount = 0;
  bool empty = true;
  commline[0] = 0;
  while( empty ) {		// stay here until something input
    if( ssSerial.available( ) > 0 ) {
      while( ssSerial.available( ) ) {
        commline[lcount] = tolower( ssSerial.read( ) );
//        ssSerial.print( commline[lcount], HEX );
        lcount++;
        if( commline[lcount-1] == '\\' ) break;
      } // while input
      if( commline[lcount-1] == '\\' ) {
        commline[lcount-1] = 0;
        lcount = 1 + strlen( commline ); // includes terminator(s)
        empty = false;
//        ssSerial.println( "" );
//        ssSerial.print( commline );
//        ssSerial.print( "  ccount " );
//        ssSerial.println( ccount );
//        ssSerial.println( "" );
      } // if end of line
    } // if input available
  } // while waiting for input
  return lcount;
} // getLine
