/* test LcdBarCentreZero_I2c library
 *
 * Created: Sept 20, 2011 - G. D. Young
 *
 * Revised: Oct 21/11 - I2C display
 *          May 31/15 - using TinyWireM library
 *
 * Draws 4 zero-centre bar graphs on 16 col 2 line display
 */

//#include <Wire.h>
#include <TinyWireM.h>          // only change required
#include <LiquidCrystal_I2C.h>
#include <LcdBarCentreZero_I2C.h>

#define MAXBARS 9       //if this is bigger on 16 col display, see
                        // interference between adjacent bargraphs

//LiquidCrystal lcd( 12, 11, 5, 4, 3, 2 ); // create LCD instance
LiquidCrystal_I2C lcd( 0x20, 16, 2 );     // create IC2 LCD instance
LcdBarCentreZero_I2C zcb( &lcd );     // create bar graph instance


byte lcdNumCols = 16;     // -- number of columns in the LCD
int barCount = 0;         // -- value to plot for this example
byte up = 1;

long interval = 150;      // interval at which to inc bar (milliseconds)
long previousMillis;
long randNr;


void setup( ){
//  lcd.begin( 2, lcdNumCols );
  lcd.init( );                  //I2C setup
  zcb.loadCG( );                // bargraph setup
  lcd.clear( );
  lcd.backlight( );
  previousMillis = millis( );    // init interval timer for drawing
}

void loop( )
{
    if( millis( ) - previousMillis > interval ) {
      // save the last time you counted
      previousMillis = millis( );   

      // count up the number of bars to draw
      if( up ) {
        barCount += 1;
        if( barCount > MAXBARS ) {
          up = 0;
          barCount = MAXBARS;
        }
      } else {              // OR, count them down
        barCount -= 1;
        if( barCount < -MAXBARS ) {
          up = 1;
          barCount = -MAXBARS;
        }
      }
      randNr = random( -MAXBARS, MAXBARS+1 );    // use random number to show blanking
      zcb.drawBar( barCount, MAXBARS, 3, 0 );
      zcb.drawBar( (int)randNr, MAXBARS, 11, 0 );
      zcb.drawBar( -barCount, MAXBARS, 4, 1 );    //short bar to show clipping to max
      zcb.drawBar( barCount, 5, 11, 1 );
      
    } // if time to count
}

