#ifndef _INCINPUTEDITORH
#define _INCINPUTEDITORH

#define DIGITS 6
#define SCALEPOS DIGITS

#define FMT_MILLIS 2
#define FMT_SECS 3
#define FMT_TIMEAUTO 1
#define FMT_BOOL 4
#define FMT_PERCENT 5
#define FMT_INT 0

class InputDial
{
private:
  Adafruit_ST7735 *screen;
  char digits[DIGITS];
  int curPos;
  char units;
  boolean isScalable;
  unsigned long value;
  void drawBoard();
  void drawUnderline();
  unsigned long digitsToValue();
  void valueToDigits(unsigned long);


public:
  InputDial();
  void setValue(unsigned long);
  void setScale(byte);
  unsigned long getValue();
  void debug();
  void init(Adafruit_ST7735*, unsigned long, unsigned int);
  void keyUp();
  void keyDown();
  void keyLeft();
  void keyRight();
  void keyPress();
};
#endif

