// testclk - use library clock routines to send date and time to serial

// created: Feb 25, 2014  G. D. (Joe) Young <jyoung@islandnet.com>

// revised: Mar 4/14 - use get's error return
//          Mar 13/14 - RV3029, overwrite time line
//          Mar 18/14 - PCF8563 version. Same source for all clocks


//#include "PCF8563.h"  // uncomment whichever lib corresponds to connected chip
#include "DS3231.h"
//#include "RV3029.h"
#include "Wire.h"

#define INTERVAL 1000		// number of msec between printing times
DS3231 clk;
//RV3029 clk;
//PCF8563 clk;

Time time;
Date date;

unsigned long timer;

void setup( ) {
  Serial.begin( 9600 );
  Wire.begin( );
  Serial.print( "begin testclk - " );
  byte err = clk.getDate( date );
  if( err != 0 ) {
    Serial.print( "read err: " );
    Serial.print( err, HEX );
    Serial.print( "   " );
  }
  Serial.print( date.yr, HEX );
  Serial.print( '/' );
  Serial.print( date.mo, HEX );
  Serial.print( '/' );
  Serial.println( date.dom, HEX );
  timer = millis( ) + INTERVAL;		//set up time printing
}


void loop( ) {
  if( timer < millis( ) ) {
    timer += INTERVAL;
    byte err = clk.getTime( time );
    if( err != 0 ) {
      Serial.print( "read err: " );
      Serial.print( err, HEX );
      Serial.print( "   " );
    }
    Serial.print( time.hr, HEX );
    Serial.print( ':' );
    Serial.print( time.min, HEX );
    Serial.print( ':' );
    Serial.println( time.sec, HEX );
  }
}

