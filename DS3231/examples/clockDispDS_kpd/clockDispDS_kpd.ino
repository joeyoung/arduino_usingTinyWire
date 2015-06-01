// clock display - with keypad input setting - clockDispDS_kpd

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
//          May 26/15 - keypad input setting

#include <Keypad_I2C.h>
#include <Keypad.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <flashPin.h>

//#include <PCF8563.h>
#include <DS3231.h>
//#include <RV3029.h>

#define LCD_ADR 0x20  // set the LCD address to 0x20
#define LCD_CHRS 16   // for a 16 chars ...
#define LCD_LINS 2    //   and 2 line display

#define KPD_ADR 0x38  // keypad 8274A base
const byte ROWS = 4;  //four rows
const byte COLS = 4;  //four columns
char keys[ROWS][COLS] = {
  {'1','2','3','+'},
  {'4','5','6','-'},
  {'7','8','9','<'},
  {'*','0','#','>'}
};
// Digitran keypad, bit numbers of PCF8574 i/o port
byte rowPins[ROWS] = {0, 1, 2, 3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 5, 6, 7}; //connect to the column pinouts of the keypad

Keypad_I2C kpd( makeKeymap(keys), rowPins, colPins, ROWS, COLS, KPD_ADR, PCF8574 );
byte handleKey( byte ikey );
void convertInput( char * buf );
char *numDate( Date &, byte * );
char *numAlarm( byte *, byte * );
void convertDateInput( byte * );
void convertAlarmInput( byte * );
byte ib[LCD_CHRS+1];     //input buffer
byte cp;                 //cursor position
char key = 0;

LiquidCrystal_I2C lcd( LCD_ADR, LCD_CHRS, LCD_LINS );
flashPin pin13;

unsigned long sectick;
byte seconds = 0;
#ifdef PCF8563_HDR
PCF8563 clk;
#endif
#ifdef DS3231_HDR
DS3231 clk;
byte alarm[ALARM1SIZE];
#endif
#ifdef RV3029_HDR
RV3029 clk;
#endif
Time time, stm;
Date date, sdt;        // now use structures for clock info
byte status[5];

void putTime( Time & );
void putDate( Date & );
const char *ver_str = "Clk0.9k";

void setup( ) {
  lcd.init();                      // initialize the lcd, wire libs
  lcd.backlight();
  kpd.begin( );
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
boolean setting = false;      // flags to control setting mode
boolean timstart = false;
boolean timeset = false;
boolean datstart = false;
boolean dateset = false;
boolean almstart = false;
boolean alarmset = false;
boolean hexin = false;
byte entwidth;

void loop()
{
  pin13.flashOff( );

  key = kpd.getKey();
  
  if( key ){
    byte err = handleKey( key );
    if( err > 2 ) {
      lcd.setCursor( 7, 0 );
      lcd.print( "error " );    // flash error message
      lcd.print( err );
      delay( 1500 );
      lcd.setCursor( 7, 0 );
      lcd.print( "        " );
      lcd.setCursor( cp, 1 );   // restore cursor to input position
   } // if error detected
  } // if key pressed
  
  if( setting ) {
    
    if( timstart ) {
      lcd.clear( );
      clk.getTime( stm );
      lcd.print( "set time" );
      putTime( stm );
      cp = 0;
      lcd.setCursor( cp, 1 );
      lcd.cursor( );
      ib[0] = 0x30 + (( stm.hr & 0x30 ) >> 4);
      ib[1] = 0x30 + (stm.hr & 0x0f);
      ib[3] = 0x30 + (( stm.min & 0xf0 ) >> 4);
      ib[4] = 0x30 + (stm.min & 0x0f);
      ib[6] = 0x30 + (( stm.sec & 0xf0 ) >> 4);
      ib[7] = 0x30 + (stm.sec & 0x0f);
      timstart = false;
    } // if initial time setting display
    
    if( datstart ) {
      lcd.clear( );
      clk.getDate( sdt );
      lcd.print( "set date" );
      numDate( sdt, ib );      // get numerical format date to ib
      cp = 0;
      lcd.setCursor( cp, 1 );
      lcd.print( (char*)ib );
      lcd.setCursor( cp, 1 );
      lcd.cursor( );
      datstart = false;
    } // if initial date setting display
    
    if( almstart ) {
      lcd.clear( );
      clk.getAlarm( 0, alarm );
      lcd.print( "set alarm" );
      numAlarm( alarm, ib );  // get numerical format alarm to ib
      cp = 0;
      lcd.setCursor( cp, 1 );
      lcd.print( (char*)ib );
      lcd.setCursor( cp, 1 );
      lcd.cursor( );
#ifdef DS3231_HDR
      entwidth = 3*ALARM1SIZE;
#else
      entwidth = 3*ALARM_SIZE;
#endif
      almstart = false;
    } // if initial alarm setting display
    
  } else {

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
  } // if setting
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


byte handleKey( byte ikey ) {
  byte returnVal = 0;
  switch( ikey ) {
    case '+':          // SET TIME
      setting = true;
      timstart = true;
      timeset = true;
      entwidth = 8;
      break;
    case '-':          // SET DATE
      setting = true;
      datstart = true;
      dateset = true;
      entwidth = 13;
      break;
    case '*':          // SET ALARM
      if( !setting ) {
        setting = true;
        almstart = true;
        alarmset = true;
      } else {         // shift toggling while setting alarm
        if( !hexin ) {
          hexin = true;  // enter hex mode
          lcd.blink( );
        } else {
          hexin = false; // exit hex mode
          lcd.noBlink( );
        } // if not hex input mode
      } // if not setting, enter setting mode
      break;
    case '<':          // CURSOR LEFT
      if( cp >= 0 ) {
        cp--;
      } else {
        cp = 0;
      }
      break;
    case '>':          // CURSOR RIGHT
      cp++;
      if( cp > LCD_CHRS-1 ) cp = LCD_CHRS-1;
      break;
    case '#':          // ENTER
      if( cp < entwidth && timeset ) {       // change wanted
        convertInput( ib );
//        lcd.setCursor( 8, 0 );
//        lcd.print( "         " );  //clear display position
//        lcd.setCursor( 8, 0 );
//        lcd.print( "=" );
//        lcd.print( stm.hr, HEX );
//        lcd.print( stm.min, HEX );
//        lcd.print( stm.sec, HEX );
//        time = stm;
        clk.setTime( stm );
        timeset = false;
      } // if time setting
      if( cp < entwidth && dateset ) {
        convertDateInput( ib );
        clk.setDate( sdt );
        dateset = false;
      } // if date setting
      if( cp < entwidth && alarmset ) {
        convertAlarmInput( ib );
        clk.setAlarm( 0, alarm );
        alarmset = false;
      }// if alarm setting
      returnVal = 1;
      setting = false;
      lcd.noCursor( );
      setup( );        //re-sync with timers after setting
      break;
//    case '*':          // CLEAR
//      lcd.setCursor( 0, 1 );
//      cp = 0;
//      for( byte ix=0; ix<LCD_CHRS; ix++ ) {
//        ib[ix] = ' ';     // fill with spaces
//      }
//      ib[LCD_CHRS] = '\0'; // make string
//      lcd.print( (char*)ib );
//      returnVal = 2;
//      break;
    default:
      if( hexin && ikey < '6' ) ikey += 0x11;
      lcd.write( ikey );
      ib[cp] = ikey;
  } // switch for specials
  lcd.setCursor( cp, 1 );
  return returnVal;
} // handleKey( )


void convertInput( byte *buf ) {
  stm.hr = ((ib[0] & 0x03)<<4) | ((ib[1] & 0x0f));
  stm.min = ((ib[3] & 0x0f)<<4) | ((ib[4] & 0x0f));
  stm.sec = ((ib[6] & 0x0f)<<4) | ((ib[7] & 0x0f));
} // convertInput( )


// Convert date to numerical format string in display image buffer
char *numDate( Date &sdt, byte *ib ) {
  if( sdt.yr < 0x80 ) {
    ib[0] = '2';
    ib[1] = '0';
  } else {
    ib[0] = '1';
    ib[1] = '9';
  }
  ib[2] = ( ( sdt.yr & 0xf0 )>>4 ) | 0x30;
  ib[3] = ( sdt.yr & 0x0f ) | 0x30;
  ib[4] = '-';
  ib[5] = ( ( sdt.mo & 0x10 )>>4 ) | 0x30;
  ib[6] = ( sdt.mo & 0x0f ) | 0x30;
  ib[7] = '-';
  ib[8] = ( ( sdt.dom&0x30 )>>4 ) | 0x30;
  ib[9] = ( sdt.dom & 0x0f ) | 0x30;
  ib[10] = ' ';
  ib[11] = ( sdt.dow & 0x0f ) | 0x30;
  ib[12] = '\0';
  return (char *)ib;
} // numDate( )


// convert text in ib[ ] to clock library Date format
void convertDateInput( byte * ib ) {
  sdt.yr = ( ( ib[2] & 0x0f )<<4 ) | ( ib[3] & 0x0f );
  sdt.mo = ( ( ib[5] & 0x0f )<<4 ) | ( ib[6] & 0x0f );
  sdt.dom = ( ( ib[8] & 0x0f )<<4 ) | ( ib[9] & 0x0f );
  sdt.dow = ( ib[11] & 0x0f );
} // convertDateInput( )


// get numerical format alarm to display/edit into ib[ ]
char *numAlarm( byte *al, byte *ib ) {
#ifdef DS3231_HDR
  for( byte ix=0; ix<ALARM1SIZE; ix++ ) {
#else
  for( byte ix=0; ix<ALARM_SIZE; ix++ ) {
#endif
  byte iy = 3*ix;
  ib[iy] = ( al[ix] & 0xf0 )>>4;
  if( ib[iy] > 9 ) ib[iy] += 0x37; else ib[iy] += 0x30;
  ib[iy+1] = ( al[ix] & 0x0f );
  if( ib[iy+1] > 9 ) ib[iy+1] += 0x37; else ib[iy+1] += 0x30;
  ib[iy+2]= ' ';
  }
#ifdef DS3231_HDR
  ib[3*ALARM1SIZE-1] = '\0';
#else
  ib[3*ALARM_SIZE-1] = '\0';
#endif
  return (char*)ib;
} // numAlarm( )


// convert (edited) text in ib to clock library alarm format
void convertAlarmInput( byte * ib ) {
#ifdef DS3231_HDR
  for( byte ix=0; ix<ALARM1SIZE; ix++ ) {
#else
  for( byte ix=0; ix<ALARM_SIZE; ix++ ) {
#endif
  byte iy = 3*ix;
  ib[iy] &= 0x4f;
  if( ib[iy] <= 9 ) alarm[ix] = ib[iy]<<4;
    else alarm[ix] = ( ( ib[iy] & 0x0f ) + 9 )<<4;
  ib[iy+1] &= 0x4f;
  if( ib[iy+1] <= 9 ) alarm[ix] |= ib[iy+1];
    else alarm[ix] |= ( ( ib[iy+1] & 0x0f ) + 9 );
  }
} // convertAlarmInput( )
