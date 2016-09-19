

//Use these pins for the shield!
#define sclk 13
#define mosi 11
#define cs   10
#define dc   8
#define rst  0  // you can also connect this to the Arduino reset


#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <MemoryFree.h>
#include <avr/pgmspace.h>
#include <limits.h>
#include "pmenu.h"
#include "pmenunode.h"
#include "dispatch.h"
#include "inputdial.h"

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif

// #define _DIAG

Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);

#define Neutral 0
#define Press 1
#define Up 2
#define Down 3
#define Right 4
#define Left 5


#define PGM_TIMELAPSE 0x01
#define PGM_LIGHTNING 0x02
#define PGM_SOUNDFLASH 0x11
#define PGM_SOUNDSHUTTER 0x12
#define PGM_SOUNDSFSEQ 0x14
#define PGM_EXTERNALFLASH 0x21
#define PGM_EXTERNALSHUTTER 0x22
#define PGM_EXTERNALSFSEQ 0x24
#define PGM_BIAS 0x40



#define MENU_PGM 0x01
#define MENU_PARAM 0x02
#define MENU_ACTION 0x03

#define CMD_SETPGM 0x01
#define CMD_SETMENU 0x02
#define CMD_SETACTION 0x04
#define CMD_SETVARINT 0x10
#define CMD_SETVARTIME 0x11
#define CMD_SETPARAM 0x20

#define ACTION_RUN 4
#define ACTION_STOP 5
#define ACTION_CALIBRATE 6
#define ACTION_CONFIGURE 7


#define ARG_SETVAR_THRESHOLD        0x001
#define ARG_SETVAR_SHUTTERDURATION  0x002
#define ARG_SETVAR_FLASHDURATION    0x004
#define ARG_SETVAR_FLASHDELAY       0x008
#define ARG_SETVAR_SIGNALDELAY      0x010
#define ARG_SETVAR_SIGNALDURATION   0x020
#define ARG_SETVAR_INTERVAL         0x040
#define ARG_SETVAR_LIMIT            0x080
#define ARG_SETVAR_RESETDELAY       0x100
#define ARG_SETVAR_AUTOFOCUS        0x200

#define INPUTSATE_EXEC 1;
#define INPUTSTATE_MENU 2;
#define INPUT_STATE_KEYS 3;

#define PIN_EXTERNALSENSOR A0   
#define PIN_LIGHTSENSOR A2    
#define PIN_SOUNDSENSOR A0 
#define PIN_SHUTTER 4
#define PIN_FOCUS 5
#define PIN_FLASH 6
#define PIN_SIGNALOUT 4
#define PIN_BREAK 9


#define BIASSAMPLESIZE 5
#define BIASINTERVAL 500

#define DISP_LINESPACE 12
#define DISP_VOFFSET 9

boolean _isRunning = false;
boolean _shutterCycling = false;

const PMenuNode PGMArray[]  = { 
  PMenuNode("TimeLapse", CMD_SETPGM, PGM_TIMELAPSE ), 
  PMenuNode("Lightning", CMD_SETPGM, PGM_LIGHTNING ),
  PMenuNode("SoundFlash", CMD_SETPGM, PGM_SOUNDFLASH ),
  PMenuNode("SoundShutter", CMD_SETPGM, PGM_SOUNDSHUTTER ),
  PMenuNode("SoundSFSeq", CMD_SETPGM, PGM_SOUNDSFSEQ ),
  PMenuNode("ExternalFlash", CMD_SETPGM, PGM_EXTERNALFLASH),
  PMenuNode("ExternalShutter", CMD_SETPGM, PGM_EXTERNALSHUTTER),
  PMenuNode("ExternalSFSeq", CMD_SETPGM, PGM_EXTERNALSFSEQ),
  PMenuNode("BiasTest", CMD_SETPGM, PGM_BIAS)

  };


const PMenuNode paramArray[]  = {
  PMenuNode("Threshold", CMD_SETVARINT, ARG_SETVAR_THRESHOLD), 
  PMenuNode("ShutterDur", CMD_SETVARTIME, ARG_SETVAR_SHUTTERDURATION),
  PMenuNode("FlashDur", CMD_SETVARTIME, ARG_SETVAR_FLASHDURATION),
  PMenuNode("FlashDelay", CMD_SETVARTIME, ARG_SETVAR_FLASHDELAY),
  PMenuNode("SignalDur", CMD_SETVARTIME, ARG_SETVAR_SIGNALDURATION), 
  PMenuNode("SignalDelay", CMD_SETVARTIME, ARG_SETVAR_SIGNALDELAY),
  PMenuNode("Interval", CMD_SETVARTIME, ARG_SETVAR_INTERVAL),
  PMenuNode("Limit", CMD_SETVARINT, ARG_SETVAR_LIMIT),
  PMenuNode("ResetDelay", CMD_SETVARTIME, ARG_SETVAR_RESETDELAY),
  PMenuNode("AutoFocus", CMD_SETVARINT, ARG_SETVAR_AUTOFOCUS),
  PMenuNode("Back", CMD_SETMENU, MENU_ACTION)
  };

const PMenuNode actionArray[]  = { 
  PMenuNode("Configure", CMD_SETACTION, ACTION_CONFIGURE), 
  PMenuNode("Calibrate", CMD_SETACTION, ACTION_CALIBRATE),
  PMenuNode("RUN", CMD_SETACTION, ACTION_RUN),
  PMenuNode("Back", CMD_SETMENU, MENU_PGM)
  };

  PMenu *activeMenu = NULL;
InputDial dial = InputDial();
Dispatch dispatcher = Dispatch();


byte _program = 0;
char * _progName;
byte _inputState = INPUTSTATE_MENU;

unsigned _threshold = 25;
unsigned _triggerLevel = INT_MIN;
unsigned _lastEvent = 0;
unsigned long _shutterDur =75;
unsigned long _flashDur =5;
unsigned long _flashDelay =0;
unsigned long _signalDur =10; 
unsigned long _signalDelay =100;
unsigned long _interval=30000;
int _limit = 0;
int _eventCount;
unsigned long _resetDelay=100;
char _autofocus = (char) false;
unsigned long _nextTimedEvent = 0;
;


// Check the joystick position
int CheckJoystick()
{
  int joystickState = analogRead(3);
  if (joystickState < 50) return Down;
  if (joystickState < 150) return Right;
  if (joystickState < 250) return Press;
  if (joystickState < 500) return Up;
  if (joystickState < 650) return Left;
  return Neutral;
}


void setup(void) {
  Serial.begin(9600);
  Serial.print("freeMemory()=");
  Serial.println(freeMemory());
#ifdef _DIAG 
  Serial.println( "STARTING...");
#endif // 
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  tft.fillScreen(ST7735_BLACK);
  //dial.init(&tft, 0);
  setMenu(MENU_PGM);
  pinMode(PIN_SHUTTER, OUTPUT);
  pinMode(PIN_FOCUS, OUTPUT);
  pinMode(PIN_FLASH, OUTPUT);
  pinMode(PIN_SIGNALOUT, OUTPUT);
  pinMode(PIN_BREAK, INPUT);
  digitalWrite(PIN_SHUTTER, LOW);
  digitalWrite(PIN_FOCUS, LOW);
  digitalWrite(PIN_FLASH, LOW);
  digitalWrite(PIN_SIGNALOUT, LOW);
}

void loop() {  
  int joy = CheckJoystick();
  if( joy != Neutral ) {
    processKey(joy);
    delay(250
      );
  }
  if (_isRunning)
  {
    switch(_program)
    {
    case PGM_TIMELAPSE:
      processEventTimed();
      break;

    case PGM_LIGHTNING:
      processEventLightning();
      break;

    case PGM_SOUNDFLASH:
    case PGM_SOUNDSHUTTER:
    case PGM_SOUNDSFSEQ:
      processEventSound();
      break;

    case PGM_EXTERNALFLASH:
    case PGM_EXTERNALSHUTTER:
    case PGM_EXTERNALSFSEQ:
      processEventExternal();
      break;

    case PGM_BIAS:
      processBiasTest();
      break;
    }
    if(digitalRead(PIN_BREAK)==LOW)
    {
      stopRunning();
    }
  }
}

void processKey(int joy)
{
  //Serial.println( "joystick pressed" );
  boolean inputMode = dispatcher.isInputMode();
  if(inputMode == false )
  {
    switch(joy)
    {
    case Up:
      activeMenu->PreviousNode();
      break;

    case Down:
      activeMenu->NextNode();
      break;

    case Press:
      activeMenu->SelectNode();

      dispatcher.debug("Press");
      processCommand(dispatcher.getCmd(), dispatcher.getArg());

      break;

    case Right:
      tft.fillScreen(ST7735_BLACK);   
      activeMenu->render();
      break;

    case Left:
      displayParams();
    }
  }
  else              // input mode is true.  Joystick used for "keyboard" input
  {
    switch(joy)
    {
    case Up:
      dial.keyUp();
      break;

    case Down:
      dial.keyDown();
      break;

    case Press:
      dial.keyPress();
      dispatcher.debug("input Press");
      processCommand(CMD_SETPARAM, dial.getValue());
      break;

    case Right:
      dial.keyRight();
      break;

    case Left:
      dial.keyLeft();
      break;
    }
  }
}

void processCommand(unsigned cmd, unsigned arg)
{
#ifdef _DIAG
  char buf[60];
  sprintf(buf, "pCom: %02x %02x", cmd,arg);
  Serial.println(buf);
#endif // _DIAG

  switch(cmd)
  {
  case CMD_SETPGM:
    dispatcher.clearCommand();
    _program = arg;
    setProgAttributes();
    setMenu(MENU_ACTION);
    break;

  case CMD_SETMENU:
    dispatcher.clearCommand();
    setMenu(arg);
    break;

  case CMD_SETVARINT: 
  case CMD_SETVARTIME:
    tft.fillScreen(ST7735_BLUE );
    dispatcher.clearCommand();
    dispatcher.setInputVar( arg );
    dispatcher.setInputMode(true);
    editParameter(arg);
    break;

  case CMD_SETPARAM: 
    setParameter(dispatcher.getInputVar(), dial.getValue());
    dispatcher.setInputMode(false);
    dispatcher.clearCommand();                // order is critical.  This must happen *after* setParameter()
    tft.fillScreen(ST7735_BLACK );
    //dispatcher.setCommand(ACTION_CONFIGURE, 0);
    setMenu(MENU_PARAM);
    break;

  case CMD_SETACTION:
    dispatcher.clearCommand();
    _eventCount = 0;
    switch(arg)
    {
    case ACTION_RUN:
      startRunning();
      //_isRunning = true;
      break;
    case ACTION_STOP:
      stopRunning();
      //_isRunning = false;
      break;
    case ACTION_CALIBRATE:
      calibrate();
      activeMenu->render();
      break;
    case ACTION_CONFIGURE:
      setMenu(MENU_PARAM);
      break;
    }
    break;
  }
}

void banner(int _colour, const char* txt )
{
  tft.fillScreen(_colour );
  tft.setCursor(0, 50 );
  tft.setTextColor(ST7735_BLACK);
  tft.print( txt );
}

void calibrate()
{
  int pin;
  switch(_program)
  {
  case PGM_LIGHTNING:
    pin = PIN_LIGHTSENSOR;
    break;

  case PGM_SOUNDFLASH:
  case PGM_SOUNDSHUTTER:
  case PGM_SOUNDSFSEQ:
    pin = PIN_SOUNDSENSOR;
    break;

  default:
    banner(ST7735_RED,"CALIBRATION ERROR");
    return;
    pin = 0;
  }
  unsigned long cumulative = 0;
  float margin = ((float) _threshold/100) + 1.0;
  tft.fillScreen(ST7735_BLACK);
  int l=1;
  drawText( "Sampling...", (l++),ST7735_CYAN);

  for (int i = 0; i < BIASSAMPLESIZE; i++ )
  {
    cumulative += ((unsigned long) analogRead(pin) );
    delay(BIASINTERVAL);
  }
  unsigned avg = ( cumulative / (unsigned long) BIASSAMPLESIZE );
  _triggerLevel = avg * margin ;
  if(_triggerLevel == avg )
    _triggerLevel+=1;
#ifdef _DIAG
  char buf[60];
  sprintf(buf,"calib: cum:=%lu  avg=%lu  trg=%u", cumulative,  (cumulative / (unsigned long) BIASSAMPLESIZE ), _triggerLevel );
  Serial.println( buf );
#endif //_DIAG

  displayParam("Avg", avg, FMT_INT, (l++));
  displayParam("Thr", _threshold, FMT_INT, (l++));
  displayParam("Trg1", _triggerLevel, FMT_INT, (l++));
  delay(2500);
  tft.fillScreen(ST7735_BLACK );
  activeMenu->render();


}

void editParameter( unsigned  paramId)
{
#ifdef _DIAG
  char buf[50];
  sprintf(buf, "##initParam: %02x", paramId );
  Serial.println( buf);
#endif // _DIAG
  switch(paramId)
  {
  case ARG_SETVAR_THRESHOLD:
    dial.init(&tft, _threshold, FMT_PERCENT );
    break;

  case ARG_SETVAR_SHUTTERDURATION:
    dial.init(&tft, _shutterDur, FMT_TIMEAUTO );
    break;

  case ARG_SETVAR_FLASHDURATION:
    dial.init(&tft, _flashDur, FMT_MILLIS );
    break;

  case ARG_SETVAR_FLASHDELAY:
    dial.init(&tft, _flashDelay, FMT_TIMEAUTO );
    break;

  case ARG_SETVAR_SIGNALDELAY:
    dial.init(&tft, _signalDelay, FMT_TIMEAUTO );
    break;

  case ARG_SETVAR_SIGNALDURATION:
    dial.init(&tft, _signalDur, FMT_TIMEAUTO );
    break;

  case ARG_SETVAR_INTERVAL:
    dial.init(&tft, _interval, FMT_TIMEAUTO );
    break;

  case ARG_SETVAR_LIMIT:
    dial.init(&tft, _limit, FMT_INT );
    break;

  case ARG_SETVAR_RESETDELAY:
    dial.init(&tft, _resetDelay, FMT_TIMEAUTO );
    break;

  case ARG_SETVAR_AUTOFOCUS:
    dial.init(&tft, _autofocus, FMT_BOOL );
    break; 
  }
}

void setParameter( unsigned paramId, unsigned long value )
{
#ifdef _DIAG
  char buf[50];
  sprintf(buf, "##setParam: %02x  %lu", paramId, value );
  Serial.println( buf);
#endif // _DIAG
  switch(paramId)
  {
  case ARG_SETVAR_THRESHOLD:
    _threshold = (unsigned) value;
    break;

  case ARG_SETVAR_SHUTTERDURATION:
    _shutterDur = value;
    break;

  case ARG_SETVAR_FLASHDURATION:
    _flashDur = value;
    break;

  case ARG_SETVAR_FLASHDELAY:
    _flashDelay = value;
    break;

  case ARG_SETVAR_SIGNALDELAY:
    _signalDelay = value;
    break;

  case ARG_SETVAR_SIGNALDURATION:
    _signalDur = value;
    break;

  case ARG_SETVAR_INTERVAL:
    _interval = value;
    break;

  case ARG_SETVAR_LIMIT:
    _limit = (int) value;
    break;

  case ARG_SETVAR_RESETDELAY:
    _resetDelay = value;
    break;

  case ARG_SETVAR_AUTOFOCUS:
    _autofocus = ( value > 0 ) ? true : false;
    break; 
  }
}

void startRunning()
{
#ifdef _DIAG
  //Serial.println( "startRunning()");
#endif // _DIAG
  _eventCount = 0;
  _nextTimedEvent = millis() + _interval;
  _isRunning = true;
  if(_shutterCycling)
  {
    shutterOpen();
  }
  showStatus();
}

void stopRunning()
{
#ifdef _DIAG
  //Serial.println( "stopRunning()");
#endif // _DIAG
  _isRunning = false;
  if(_shutterCycling)
  {
    shutterClose();
  }
  showStatus();
  delay(2000);
  tft.fillScreen(ST7735_BLACK );
  activeMenu->render();
} 
void processEventTimed()
{
  if( millis() >= _nextTimedEvent )
  {
    _nextTimedEvent= millis() + _interval;  // reset first!
    //Serial.println("Timed Event!" );  
    shutterCycle();
    postEvent();
  }
}

void processEventLightning()
{
  unsigned sensorValue = (unsigned) analogRead(PIN_LIGHTSENSOR); 
#ifdef _DIAG
  char buf[60];
  sprintf(buf, "lighning: tLevel=%u  measure=%u",_triggerLevel,sensorValue );
  Serial.println( buf );
#endif // _DIAG

  if ( sensorValue >= _triggerLevel ) {
    _lastEvent = sensorValue;
    //Serial.println("Lightning Event!" );  
    shutterCycle();
    postEvent();
  }
}
void processEventSound()
{
  unsigned sensorValue = (unsigned) analogRead(PIN_SOUNDSENSOR); 
#ifdef _DIAG
  char buf[60];
  sprintf(buf, "sound: tLevel=%u  measure=%u",_triggerLevel,sensorValue );
  Serial.println( buf );
#endif // _DIAG

  //if ( sensorValue >= _triggerLevel ) {
    if ( sensorValue <= 100 ) {
    //Serial.println( "Sound Event!" );
    if (_program == PGM_SOUNDSHUTTER)
      shutterCycle();
    else 
    {
      flashCycle();
      postEvent();
    }
  }
}
void processEventExternal()
{
  if( (analogRead(PIN_EXTERNALSENSOR )) == 0 )
  {
    //Serial.println("External Event!" );
    if(_program == PGM_EXTERNALSHUTTER)
      shutterCycle();
    else
      flashCycle();
    postEvent();
  }
}

#define BIASSAMPLESIZE 25000
#define BIASINTERVAL 0

void processBiasTest()
{
  unsigned  min = UINT_MAX;
  unsigned  max = 0;
  unsigned long cumulative = 0;
  unsigned  val =0;
  int l = 1;
  tft.fillScreen(ST7735_BLACK);
  drawText( "Sampling...", (l++),ST7735_CYAN);
  analogRead(PIN_SOUNDSENSOR);
  for (int i = 0; i < BIASSAMPLESIZE; i++ )
  {
    val = (unsigned)analogRead(PIN_LIGHTSENSOR);
    cumulative += val;
    if (val < min && val > 0)
      min = val;
    if (val > max)
      max = val;
    if (BIASINTERVAL > 0)
      delay(BIASINTERVAL);
  }

  float margin = ((float) _threshold/100) + 1.0;
  unsigned avg = (unsigned) ( cumulative / (unsigned long)BIASSAMPLESIZE );
  _triggerLevel = avg * margin ;
  if(_triggerLevel == avg )
    _triggerLevel+=5;
  displayParam("Min", min, FMT_INT, (l++));
  displayParam("Max", max, FMT_INT, (l++));
  displayParam("Avg", avg, FMT_INT, (l++));
  displayParam("Thr", _threshold, FMT_INT, (l++));
  displayParam("Trg1", _triggerLevel, FMT_INT, (l++));
  if(_triggerLevel == avg )
    _triggerLevel+=5;
  displayParam("Trg2", _triggerLevel, FMT_INT, (l++));

  delay(2500);
}

void drawText(char *text, int line, uint16_t color) {
  tft.setCursor(0, line*DISP_LINESPACE+DISP_VOFFSET);
  tft.setTextColor(color);
  tft.print(text);
}

void shutterCycle() 
{
  shutterOpen();
  /*
  tft.fillScreen(ST7735_RED);  
   tft.setCursor(0, 20);
   tft.setTextColor(ST7735_BLACK);
   tft.print(arg);
   */
  delay(_shutterDur);
  tft.fillScreen(ST7735_BLACK);
  shutterClose();
}

void shutterOpen()
{
  if(_autofocus == true )
    focus();

#ifdef _DIAG
  Serial.println( "Shutter OPEN" );
#endif // _DIAG

  digitalWrite(PIN_SHUTTER, HIGH);
}
void focus()
{
#ifdef _DIAG
  Serial.println( "focussing" );
#endif // _DIAG

  digitalWrite(PIN_FOCUS, HIGH);
  delay(500);
  digitalWrite(PIN_FOCUS, LOW);

}

void shutterClose()
{
#ifdef _DIAG
  Serial.println( "Shutter CLOSE" );
#endif // _DIAG
  digitalWrite(PIN_SHUTTER, LOW);
}

void flashCycle()
{
#ifdef _DIAG
  Serial.println( "Flash!" );
#endif // _DIAG
  digitalWrite(PIN_FLASH, HIGH);
  if( _flashDur > 0 )
    delay(_flashDur);
  digitalWrite(PIN_FLASH, LOW );
}

void displayParam(char* pName, unsigned long pVal, byte pFmt, int line )
{

  char buf[64];
  switch( pFmt )
  {
  case FMT_MILLIS:
    sprintf(buf, "%s = %lu mS", pName, pVal );
    break;
  case FMT_SECS:
    sprintf(buf, "%s = %lu S", pName, pVal/1000 );
    break;
  case FMT_TIMEAUTO:
    if(pVal >= 60000)
      sprintf(buf, "%s = %lu S", pName, pVal/1000 );
    else
      sprintf(buf, "%s = %lu mS", pName, pVal );
    break;
  case FMT_PERCENT:
    sprintf(buf, "%s = %lu %%", pName, pVal );
    break;
  case FMT_BOOL:
    sprintf(buf,"%s = %s", pName, (_autofocus) ? "yes" : "no" );
    break;
  case FMT_INT:
  default:
    sprintf(buf, "%s = %lu", pName, pVal );
    break;
  }
  drawText( buf, line,ST7735_CYAN);
}

void displayParams()
{
  int l=0;
  char buf[32];
  tft.fillScreen(ST7735_BLACK);
  drawText( "PARAMETERS", (l++),ST7735_CYAN);
  displayParam("threshold", _threshold, FMT_PERCENT, (l++));
  displayParam("triggerLevel", _triggerLevel, FMT_PERCENT, (l++));

  displayParam("shutterDur", _shutterDur, FMT_TIMEAUTO,(l++));
  displayParam("flashDelay",_flashDelay, FMT_TIMEAUTO,(l++));
  displayParam("flashDur", _flashDur, FMT_MILLIS,(l++));
  displayParam("signalDur",  _signalDur, FMT_TIMEAUTO,(l++));
  displayParam("signalDelay", _signalDelay, FMT_TIMEAUTO,(l++));
  displayParam("interval", _interval, FMT_TIMEAUTO,(l++));
  displayParam("limit", _limit, FMT_INT,(l++));
  displayParam("resetDelay", _resetDelay, FMT_TIMEAUTO,(l++));
  displayParam("auttofocus", _autofocus, FMT_BOOL,(l++));
  delay(4000);
  tft.fillScreen(ST7735_BLACK); 
  activeMenu->render();
}


void setMenu(int menuKey)
{
  activeMenu = NULL;  
  tft.fillScreen(ST7735_BLACK);

  switch(menuKey)
  {
  case MENU_PGM:
    activeMenu = new PMenu(PGMArray,(sizeof(PGMArray)/sizeof(PGMArray[0])), (const char*) "SELECT PROGRAM", &tft,&dispatcher );

    break;
  case MENU_PARAM:
    activeMenu = new PMenu(paramArray,(sizeof(paramArray)/sizeof(paramArray[0])),(const char*) "EDIT PARAMETER", &tft,&dispatcher);

    break;
  case MENU_ACTION:
    activeMenu = new PMenu(actionArray,(sizeof(actionArray)/sizeof(actionArray[0])),(const char*) "SELECT ACTION", &tft,&dispatcher);
    break;
  }
  activeMenu->render();

} 




void setProgAttributes()
{
  _shutterCycling = false;
  switch(_program)
  {
  case PGM_TIMELAPSE:
    _progName = "TL";
    break;

  case PGM_LIGHTNING:
    _progName = "LT";
    break;

  case PGM_SOUNDFLASH:
    _progName = "SF";
    break;

  case PGM_SOUNDSHUTTER:
    _progName = "SS";
    break;

  case PGM_SOUNDSFSEQ:
    _progName = "SSF";
    _shutterCycling = true;
    break;

  case PGM_EXTERNALFLASH:
    _progName = "EF";
    break;
  case PGM_EXTERNALSHUTTER:
    _progName = "ES";
    break;
  case PGM_EXTERNALSFSEQ:
    _progName = "ESF";
    _shutterCycling = true;
    break;

  default:
    _progName = "UNKNOWN";
    break;
  }
}

void showStatus()
{
  char buf[60];
  int line=0;
  tft.fillScreen(ST7735_BLACK);

  drawText( "STATUS", line++,ST7735_CYAN);
  sprintf( buf, "%s", _progName );
  drawText( buf, line++,ST7735_WHITE);
  sprintf( buf, "State: %s", (_isRunning) ? "Running" : "STOPPED!" );
  drawText( buf, line++,ST7735_WHITE);
  if( _limit > 0 )
    sprintf( buf, "%d of %d events ", _eventCount, _limit );
  else
    sprintf( buf, "%d events", _eventCount );
  drawText( buf, line++,ST7735_WHITE);
  sprintf( buf, "trig: %u", _triggerLevel);
  drawText( buf, line++,ST7735_WHITE);
  sprintf( buf, "last: %u", _lastEvent );
  drawText( buf, line++,ST7735_WHITE);

}

void postEvent()
{
  _eventCount++;
  showStatus();
  if( _limit > 0 && _eventCount >= _limit )
    stopRunning();
  if( _shutterCycling )
  {
    // close then reopen shutter
    shutterClose();
    shutterOpen();
  }
  if(_resetDelay > 0 )
    delay(_resetDelay); 
}




