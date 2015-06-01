// getKeyLcd - editing input from I2C keypad displayed on I2C LCD
//
// started: May 25, 2015  G. D. (Joe) Young <jyoung@islandnet.com>
//
// revised: May 26/15 - add int input conversion
//          May 28/15 - hex input with shift key 
//          May 31/15 - tinywire version
//
// Obtain user input from keypad, allowing for editing the entry while
// display shows the current value of input. Since display does not support
// reading the displayed value, a display image buffer is maintained. Editing
// method is to allow (repeated) keypad entry at a given display cursor
// position until cursor is moved to another position. Editing termination is
// indicated when an 'enter' key is detected.
//
//

#include <Keypad_tw.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <TinyWireM.h>

#define I2CADDR 0x38  // keypad on PCF8574A
#define LCD_ADR 0x20  // set the LCD address to 0x20
#define LCD_CHRS 16   // for a 16 chars ...
#define LCD_LINS 2    //   and 2 line display

LiquidCrystal_I2C lcd( LCD_ADR, LCD_CHRS, LCD_LINS );

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {'1','2','3','+'},
  {'4','5','6','-'},
  {'7','8','9','<'},
  {'*','0','#','>'}
};
// Digitran keypad, bit numbers of PCF8574 i/o port
byte rowPins[ROWS] = {0, 1, 2, 3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 5, 6, 7}; //connect to the column pinouts of the keypad

Keypad_tw kpd( makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2CADDR, PCF8574 );

char ib[LCD_CHRS+1];     //input buffer
byte cp;                 //cursor position
char key = 0;
int var_a, var_b;        //result variable

byte handleKey( byte ikey );
int convertInput( char * buf );

byte ledpin = 13;

void setup() {
//  Wire.begin( );
  kpd.begin( );
  lcd.init( );
  lcd.backlight();
  lcd.print( "var A" );
  lcd.setCursor( 0, 1 );
  lcd.cursor( );
  pinMode( ledpin, OUTPUT );
} // setup( )

void loop( ) {
  
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

} // loop( )


boolean hexin;

byte handleKey( byte ikey ) {
  byte returnVal = 0;
  switch( ikey ) {
    case '+':          // TOGGLE SHIFT
      if( !hexin ) {
        lcd.blink( );
        hexin = true;
        digitalWrite( ledpin, HIGH );
      } else {
        hexin = false;
        lcd.noBlink( );
        digitalWrite( ledpin, LOW );
      }
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
      ib[cp+1] = '\0';   // make input buffer a string
//      var_a = convertInput( ib );
      lcd.setCursor( 6, 0 );
      lcd.print( "          " );  //clear display position
      lcd.setCursor( 6, 0 );
      lcd.print( "= " );
//      lcd.print( var_a );
      lcd.print( ib );
      returnVal = 1;
      break;
    case '*':          // CLEAR
      lcd.setCursor( 0, 1 );
      cp = 0;
      for( byte ix=0; ix<LCD_CHRS; ix++ ) {
        ib[ix] = ' ';     // fill with spaces
      }
      ib[LCD_CHRS] = '\0'; // make string
      lcd.print( ib );
      returnVal = 2;
      break;
    default:
      if( hexin && ikey < '6' ) ikey += 0x11;
      lcd.write( ikey );
      ib[cp] = ikey;
  } // switch for specials
  lcd.setCursor( cp, 1 );
  return returnVal;
} // handleKey( )


int convertInput( char *buf ) {
  return( atoi( buf ) );
} // convertInput( )
