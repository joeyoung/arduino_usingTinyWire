// clock display

// created: Oct 12, 2013 G. D. (Joe) Young <jyoung@islandnet.com>

// revised: Oct 15 - sync read to clock chip's seconds
//          Oct 17 - calendar display
//          Oct 18 - put date each minute, temp display
//          Oct 20 - rearrange display
//          Mar 08/14 - change to use clock libraries
//          Mar 10/14 - DS3231 version called clockDispDS
//                    - conditional compile - only need to change
//                      the #include line to switch chips
//          Mar 11/14 - add RV3029
//          Mar 17/14 - use revised getStatus, setStatus
//          Mar 17/14 - DS version with alarm1 flashing
//          Mar 18/14 - PCF version



#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <flashPin.h>

//#include <PCF8563.h>
#include <DS3231.h>
//#include <RV3029.h>

#define LCD_ADR 0x20  // set the LCD address to 0x20
#define LCD_CHRS 16   // for a 16 chars ...
#define LCD_LINS 2    //   and 2 line display

LiquidCrystal_I2C lcd( LCD_ADR, LCD_CHRS, LCD_LINS );
flashPin pin13;

unsigned long sectick;
byte seconds = 0;
#ifdef PCF8563_HDR
PCF8563 clk;
#endif
#ifdef DS3231_HDR
DS3231 clk;
#endif
#ifdef RV3029_HDR
RV3029 clk;
#endif
Time time;
Date date;        // now use structures for clock info
byte status[5];

void putTime( Time & );
void putDate( Date & );
const char *ver_str = "Clk 0.9";

void setup( ) {
  lcd.init();                      // initialize the lcd, wire libs
  lcd.backlight();
  byte errCode = clk.getTime( time );
  if( errCode == RTC_OK ) {
    seconds = time.sec;          // sync up with clock chip
    while( seconds == time.sec ) {
      clk.getTime( time );
    } // wait for clock chip to change
  } // if clock read is ok
  sectick = millis( ) + 1000L;
  clk.getTime( time );
  putTime( time );
  clk.getDate( date );
  date.dow &= 0x07;  // ????? this needs fixing
  putDate( date );
  lcd.setCursor( 9, 1 );
  lcd.print( ver_str );
// handle different alarm setups for each clock
#ifdef RV3029_HDR
  clk.getStatus( RVSTAT0, 5, status );  //enable alarms
  status[RVSTAT1] |= (1<<AIE);
  clk.setStatus( RVSTAT1, status[RVSTAT1] );
#endif
#ifdef DS3231_HDR
  byte statflags;
  clk.getStatus( DSSTATUS+1, &statflags );
  statflags &= ~(1<<A1F);   // clear alarm1 flag
  clk.setStatus( DSSTATUS+1, statflags );
#endif
#ifdef PCF8563_HDR
  byte statflags;
  clk.getStatus( PCSTATUS+1, &statflags ); //read flags byte
  statflags &= ~(1<<AF);  //clear alarm flag
  clk.setStatus( PCSTATUS+1, statflags );  //write flags back
#endif
}

boolean alarmFlashing = false;

void loop()
{
  pin13.flashOff( );

  if( sectick < millis( ) ) {
    sectick += 1000L;
    time.sec++;
    if( ( time.sec & 0x0f ) > 9 ) {
      time.sec &= 0xf0;
      time.sec += 0x10;
    }

    if( alarmFlashing && time.sec <= 0x15 ) pin13.flash( );
    
    if( time.sec == 0x60 ) {
      clk.getTime( time );
      putTime( time );
      clk.getDate( date );
      putDate( date );
#ifdef PCF8563_HDR
      lcd.setCursor( 9, 1 );
      lcd.print( "        " );
      clk.getStatus( PCSTATUS, 2, status );
      if( status[1] & (1<<AF) ) {
        status[1] &= ~(1<<AF);
        clk.setStatus( PCSTATUS, 2, status );
        pin13.flash( );
        alarmFlashing = true;
      } else {
        alarmFlashing = false;
      } // if alarm flag bit set
#endif
// for chips with a temp sensor (DS3231, RV3029)
#ifdef RV3029_HDR
      byte temp = 0;
      clk.getStatus( RVSTATMP, &temp );
      putTemp( &temp );
      clk.getStatus( RVSTAT2, status+2 );
      if( status[2]&(1<<AF) ) {  // check alarm
        pin13.flash( );
        status[2] &= ~(1<<AF);  // clear AF
        clk.setStatus( RVSTAT2, status[2] );
        alarmFlashing = true;
      } else {
        alarmFlashing = false;
      } // if alarm
#endif
#ifdef DS3231_HDR
      clk.getStatus( DSSTATMP, 2, status );
      putTemp( status );
      clk.getStatus( DSSTATUS, 2, status );
      if( status[1]&(1<<A1F) ) { //check alarm1
        pin13.flash( );
        status[1] &= ~(1<<A1F);  // clear A1F
        clk.setStatus( DSSTATUS+1, status[1] );
        alarmFlashing = true;
      } else {
        alarmFlashing = false;
      } // if alarm
#endif
    } // each minute read clock, rewrite display
    putTime( time );
  } // counting seconds

} // loop

const char days[7][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
const char months[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                             "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

void putTime( Time &tim ) {
  lcd.setCursor( 0, 1 );
  lcd.write( ( ( tim.hr & 0xf0 )>>4 ) + 0x30 );
  lcd.write( ( tim.hr & 0x0f ) + 0x30 );
  lcd.write( ':' );
  lcd.write( ( ( tim.min & 0xf0 )>>4 ) + 0x30 );
  lcd.write( ( tim.min & 0x0f ) + 0x30 );
  lcd.write( ':' );
  lcd.write( ( ( tim.sec & 0xf0 )>>4 ) + 0x30 );
  lcd.write( ( tim.sec & 0x0f ) + 0x30 );
} // putTime

void putDate( Date &dat ) {
  lcd.setCursor( 0, 0 );
  lcd.print( days[dat.dow-1] );   // be aware of clock chips dow: 0..6, or 1..7
  lcd.write( ' ' );
  byte bixmo = ( ((dat.mo&0x10)>>4)*10 ) + ( date.mo&0x0f );
  lcd.print( months[bixmo-1] );
  lcd.write( ' ' );
  lcd.write( ( ( dat.dom&0x30 )>>4 ) + 0x30 );
  lcd.write( ( ( dat.dom&0x0f ) + 0x30 ) );
  lcd.write( ' ' );
  if( dat.yr < 0x80 ) {
    lcd.print( "20" );
  } else {
    lcd.print( "19" );      // century kludge
  }
  lcd.write( ( ( dat.yr & 0xf0 )>>4 ) + 0x30 );
  lcd.write( ( ( dat.yr & 0x0f ) + 0x30 ) );
} // putDate

#ifdef DS3231_HDR
void putTemp( byte *tregs ) {
  lcd.setCursor( 9, 1 );
  byte chrs = lcd.print( tregs[0], DEC );      //msb of temp
  chrs += lcd.write( '.' );
  chrs += lcd.print( 25*(tregs[1]>>6), DEC );  //fractional temp, .25 degree resolutn
  chrs += lcd.write( 0xdf );
  while( chrs < 7 ) {
	  lcd.write( ' ' );
	  chrs++;
  } // fill out with spaces
} // putTemp( )
#endif

#ifdef RV3029_HDR
void putTemp( byte *tregs ) {
  lcd.setCursor( 9, 1 );
//  lcd.print( "temp " );
//  lcd.print( tregs[0], HEX );
  int temp = (int)tregs[0] - 60;
  byte chrs = lcd.print( temp, DEC );
  chrs += lcd.write( 0xdf );
  chrs += lcd.write( 'C' );
  while( chrs < 7 ) {
	  lcd.write( ' ' );
	  chrs++;
  } // fill out with spaces
} // putTemp( ) - for RV3029
#endif
