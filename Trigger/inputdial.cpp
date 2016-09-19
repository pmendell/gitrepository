#include <SPI.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include "inputdial.h"


//Use these pins for the shield!
#define sclk 13
#define mosi 11
#define cs   10
#define dc   8
#define rst  0  // you can also connect this to the Arduino reset

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#define dPadOriginX 110
#define dPadOriginY 30
#define dPadWidth 14
#define dPadHeight 18
#define dPadSpace 3 



InputDial::InputDial()
{

  screen = NULL;
  value = 0;
  curPos = 0;
  isScalable = false;
  char unit = ' ';
}

unsigned long InputDial::getValue()
{

  return digitsToValue();
}
void InputDial::setValue(unsigned long v)
{
  valueToDigits(v);
}
void InputDial::keyRight()
{

  Serial.println("Right");
  if (curPos > 0 )
    curPos--;
  drawUnderline();
}

void InputDial::keyLeft()
{
  if (curPos < DIGITS-1 || isScalable && curPos < DIGITS )
    curPos++;
  drawUnderline();
}
void InputDial::keyUp()
{    
  if(curPos==SCALEPOS && isScalable )
  {
    units = (( units == 'm' ) ? 'S' : 'm');
    screen->drawChar( dPadOriginX - (SCALEPOS)*(dPadWidth+dPadSpace)+2, dPadOriginY+2, units, ST7735_BLACK, ST7735_YELLOW, 2 );
  }
  else
  {
    (digits[curPos])++;
    if(digits[curPos]>9)
      digits[curPos]=0;
    screen->drawChar( dPadOriginX - curPos*(dPadWidth+dPadSpace)+2, dPadOriginY+2, (char) '0'+digits[curPos], ST7735_BLACK, ST7735_GREEN, 2 );  
  }
}

void InputDial::keyDown()
{
  if(curPos==SCALEPOS  )
  {
    char buf[60];
    sprintf( buf, "got here keyDown units=%", units );
    Serial.println( buf);
    units = (( units == 'm' ) ? 'S' : 'm');
    screen->drawChar( dPadOriginX - (SCALEPOS)*(dPadWidth+dPadSpace)+2, dPadOriginY+2, units, ST7735_BLACK, ST7735_YELLOW, 2 );
  }
  else
  {
    (digits[curPos])--;
    if(digits[curPos]<0)
      digits[curPos]=9;
    screen->drawChar( dPadOriginX - curPos*(dPadWidth+dPadSpace)+2, dPadOriginY+2, (char) '0'+digits[curPos], ST7735_BLACK, ST7735_GREEN, 2 );  
  }
}

void InputDial::keyPress()
{
  // never called
}

void InputDial::init(Adafruit_ST7735 *sPtr, unsigned long initVal, unsigned int format)
{
  screen = sPtr;
  curPos=0;
  isScalable = false;
  switch( format )
  {
  case FMT_MILLIS:
    units='m';
    break;
  case FMT_SECS:
    units='S';
    break;
  case FMT_TIMEAUTO:
    isScalable = true;
    if( initVal > 60000 )
    {
      initVal = initVal / 1000;
      units = 'S';
    }
    else
    {
      units = 'm';
    }
    break;
  case FMT_PERCENT:
    units = '%';
    break;
  case FMT_BOOL:
  case FMT_INT:
  default:
    units = ' ';
    break;
  }
  setValue(initVal);
  drawBoard();
}

void InputDial::valueToDigits(unsigned long val )
{
  char buf[64];
  int d;
  sprintf(buf, "VTD!!: d=%d val=%lu", d, val);
  Serial.println(buf);

  unsigned long multiplier = 100000;   // this assumes DIGITS=.  Should be dynamic!
  for(int i = DIGITS-1; i>=0; i--)
  {
    d=int(val/multiplier);
    digits[i]=d;
    sprintf(buf, "VTD##: d=%d val=%lu m=%lu", d, val,multiplier);
    Serial.println(buf);
    val-=(d*multiplier);
    multiplier/=10;
  }
  for(int j=0; j<DIGITS;j++)
    Serial.println( digits[j]);
}

unsigned long InputDial::digitsToValue()
{
  char buf[60];
  /* multiply powers of 10 explicitly to avoid precision errors with pow() */
  unsigned long val = (unsigned long) 0;
  unsigned long multiplier = (unsigned long) 1;
  for(int i =0; i<DIGITS;i++)
  {
    val+= ( digits[i] * multiplier);
    //Serial.println(multiplier);
    multiplier*=(unsigned long)10;
  }
  if( isScalable && units == 'S' ) {
    val*=(unsigned long)1000;
  }
  sprintf(buf,"digitsToValue: %lu  %c", val, digits[SCALEPOS] );

  Serial.println( buf );
  value = val;
  return val;
}

#define dPadOriginX 110
#define dPadOriginY 30
#define dPadWidth 14
#define dPadHeight 18
#define dPadSpace 3

void InputDial::drawBoard() {

  screen->fillScreen(ST7735_BLUE);
  for( int i = 0; i < DIGITS; i++)
  {
    screen->fillRoundRect( dPadOriginX - i*(dPadWidth+dPadSpace), dPadOriginY, dPadWidth, dPadHeight, 5, ST7735_GREEN);
  }
  screen->fillRoundRect( dPadOriginX - (DIGITS)*(dPadWidth+dPadSpace), dPadOriginY, dPadWidth, dPadHeight, 5, ST7735_YELLOW);

  for(int i = 0; i < DIGITS; i++)
  {
    screen->drawChar( dPadOriginX - i*(dPadWidth+dPadSpace)+2, dPadOriginY+2, (char) '0'+digits[i], ST7735_BLACK, ST7735_GREEN, 2 );
  }

  screen->drawChar( dPadOriginX - (SCALEPOS)*(dPadWidth+dPadSpace)+2, dPadOriginY+2, units, ST7735_BLACK, ST7735_YELLOW, 2 );

  drawUnderline();
  //delay(100);

}

void InputDial::drawUnderline()
{

  screen->fillRect(0, 55, 128, 5, ST7735_BLUE);
  screen->fillRect( dPadOriginX - (curPos*(dPadWidth+dPadSpace)), 55, dPadWidth, 4, ST7735_RED);

}









