#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
//#define BLYNK_DEBUG
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>

#define PIN D6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across 
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

#define STRIPLEN 60
#define TRENDLEN 10
#define SOUNDLAG 250  // time in mSec post noise for flashing to quiesce
#define MODE_CYCLE 1
#define MODE_SOLID 0
#define NET_MOBILE 0
#define NET_FIXED 1
#define BLYNK_AUTH "6bc3a6bb1c59447eba8dd36362ad8e19"
#define SSID_MOBILE "<INSERT_SSID>"
#define PW_MOBILE "<INSERT_PASSWORD>"
#define SSID_FIXED "<INSERT_SSID>"
#define PW_FIXED "<INSERT_PASSWORD>"
int _trend[TRENDLEN+1];
int _sampleIdx = 0;
int _maxSoundLevel = 0;
int _lastTick = 0;
int _paramSoundThreshold = 5;
int _paramCycleSpeed = 6;
int _paramMode = MODE_SOLID;
int _paramRed = 0;
int _paramGreen = 0;
int _paramBlue = 0;
uint32_t _solidColour = strip.Color(0,0,0);
boolean _noisy = false;
unsigned long _gapMillis = 0;
unsigned long _baseMillis = 0;
int _leftbound = 0;
int _rightbound = STRIPLEN-1;
char logicalStrip[STRIPLEN];
uint32_t _wheelArray[STRIPLEN];
int _offset=0;

SimpleTimer timer1;
SimpleTimer timer2;
SimpleTimer timer3;


int sample() {
  int retVal = 0;
  char buf[512];     
  int val = analogRead(A0);
  if( val > _maxSoundLevel )
    _maxSoundLevel=val;
  float avg = 0.0;
  _trend[_sampleIdx] = val;
  for(int i=0; i < TRENDLEN; i++) {
    avg += _trend[i];
  }
  avg = avg / (float) TRENDLEN;
  float thresh = (float) _paramSoundThreshold / (float) 1000.0;
  
  if( (float)val > (avg*(1.0+thresh)) ) {


      //sprintf( buf, "**********   val=%d  avg=%d  max=%d", val, (int)avg, _maxSoundLevel);
      //Serial.println( buf);
    retVal = abs(val - avg);
  }

  if((_sampleIdx++) >= TRENDLEN ) {
     _sampleIdx=0;
     _maxSoundLevel = 0;
     //Serial.println(".");
  }
  
  return retVal;
}



void logical_show(char logicalStrip[])
{
  char printable[STRIPLEN+1];
  int i;
  uint32_t cExcept = strip.Color(255,255,255);
  for(i=0; i< STRIPLEN; i++)
  {
    if( _noisy && logicalStrip[i] == (char) 0 )
    {
      printable[i] = '0';
      strip.setPixelColor(i,cExcept);
    } else {
      printable[i] = ' ';
      strip.setPixelColor(i,(_paramMode == MODE_CYCLE) ? _wheelArray[i] : _solidColour );
    }
  }
  printable[STRIPLEN] = '\0';
  //Serial.printf("%s\n", printable);
  strip.show();
}

void clear(char logicalStrip[])
{
  int i;
  for(i=0; i<STRIPLEN; i++)
    logicalStrip[i]=0;
}

int boundsOk()
{
  
  int retVal = 1;
  if( _leftbound < 0 )
  {
      _leftbound=0;
      retVal = 0;
  }
  if( _rightbound > STRIPLEN )
  {
    _rightbound = STRIPLEN;

    retVal = 0;
  }
  if( _leftbound >STRIPLEN )
  {
      _leftbound=STRIPLEN-3;
      retVal = 0;
  }
  if( _rightbound < 0 )
  {
    _rightbound = 3;
    retVal = 0;
  }
  return retVal;
}

void set(char logicalStrip[])
{
  if(boundsOk() > 0 )
  {
  int i;
    clear(logicalStrip);
    for(i=0; i<_leftbound; i++)
      logicalStrip[i]=1;
    for(i=STRIPLEN-1;i>_rightbound;i--)
      logicalStrip[i]=1;
    logical_show(logicalStrip);
  }
}

void gap(char logicalStrip[], int len)
{
  int i;
  int startGap = _rightbound - _leftbound;
  int gapDiff = startGap - len;
  //printf("INITGAP: %d %d %d %d %d\n", len, _leftbound, _rightbound, startGap, gapDiff); 
  if( boundsOk()> 0)
  {
    if( gapDiff > 0 )
    {
      for(i=0; i<=gapDiff; i+=2) {
        _leftbound++;
        _rightbound--;
        //printf("GAP+: %d %d %d %d\n", _leftbound,  _rightbound, startGap, gapDiff); 
        set(logicalStrip);
      }
    } else {
      for(i=gapDiff; i<=0; i+=2) {
        _leftbound--;
        _rightbound++;
        //printf("GAP-: %d %d %d %d\n", _leftbound, _rightbound, startGap, gapDiff); 
        set(logicalStrip);
      }
    }
    if( (_rightbound - _leftbound) < len ){
      //printf("ADJUSTMENT\n");
      _rightbound = _leftbound + (len-1);
     set(logicalStrip);
    }
  }
}
int Min(int left, int right)
{
  return( (left <= right) ? left : right) ;
}

int Max(int left, int right)
{
  return( (left >= right) ? left : right );
}

void shift(char logicalStrip[], int amt)
{
  //Serial.println( "shifting");
  int gap = _rightbound - _leftbound;
  int i;
  int magnitude;
  if(amt < 0 )
  {
    magnitude = Max( amt, 0 );
    for(i=0; i < (-1*amt); i++)
    {
      _leftbound--;
      _rightbound--;
      set(logicalStrip);
    }
  } else {
    magnitude=Min(amt, STRIPLEN );
    for(i=0; i < amt; i++)
    {
      _leftbound++;
      _rightbound++;
      set(logicalStrip);
    }
  }
}
    
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


void initWheel(uint16_t offset) {
  uint16_t i, j;
  uint32_t c;

    for(i=0; i<strip.numPixels(); i++) {
      c=Wheel((i+offset) & 255);
      _wheelArray[i] = c;
    }
}
BLYNK_CONNECTED() {
      Serial.printf("*** %d\n", millis() );
      Blynk.syncAll();
      _solidColour = strip.Color(_paramRed,_paramGreen,_paramBlue);
      Serial.printf( "speed=%d sens=%d R=%d G=%d B=%d mode=%d\n", _paramCycleSpeed, _paramSoundThreshold, _paramRed, _paramGreen, _paramBlue, _paramMode);
      Serial.println("Syncing");
}

BLYNK_WRITE(V0) { 
 _paramCycleSpeed = param.asInt();
 Serial.printf( "new _paramCycleSpeed = %d\n", _paramCycleSpeed );
}
BLYNK_WRITE(V1) { 
 _paramSoundThreshold = param.asInt();
  Serial.printf( "new _paramSoundThreshold = %d\n", _paramSoundThreshold );
}
BLYNK_WRITE(V2) { 
 _paramRed = param.asInt();
 _solidColour = strip.Color(_paramRed,_paramGreen,_paramBlue);
}
BLYNK_WRITE(V3) { 
 _paramGreen = param.asInt();
 _solidColour = strip.Color(_paramRed,_paramGreen,_paramBlue);
}
BLYNK_WRITE(V4) { 
 _paramBlue = param.asInt();
 _solidColour = strip.Color(_paramRed,_paramGreen,_paramBlue);
}
BLYNK_WRITE(V5) { 
 _paramMode = param.asInt();
 if( _paramMode == MODE_SOLID )
  _solidColour = strip.Color(_paramRed,_paramGreen,_paramBlue);
  Serial.printf( "new _paramMode = %d\n", _paramMode );
}


void connect()
{
  // See if pins 3 and 5 are connected.  
  // If so, assume mobile, otherwise, assume home

  Serial.println("About to connect");
  pinMode(D5, OUTPUT);
  pinMode(D3, INPUT_PULLUP);
  digitalWrite(D5, LOW);  // force pin5 LOW
  int mode = digitalRead(D3);
  //char ssid[] = (mode == NET_MOBILE) ? SSID_MOBILE : SSID_FIXED;
  //char pass[] = (mode == NET_MOBILE) ? PW_MOBILE : PW_FIXED;
  Blynk.begin( BLYNK_AUTH,
  (mode == NET_MOBILE) ? SSID_MOBILE : SSID_FIXED,
  (mode == NET_MOBILE) ? PW_MOBILE : PW_FIXED
  );
  while (Blynk.connect() == false) {
    Serial.println("Waiting..." );
  }

}


void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  randomSeed(analogRead(0));
  initWheel(0);
  _baseMillis = millis();
  uint32_t cOff = strip.Color(50,0,50);
  for(int i=0; i< STRIPLEN; i++)
  {
    strip.setPixelColor(i,cOff);
    logicalStrip[i] = (char) 1;
  }
  strip.show();
  connect();

}

void loop() {


  int shiftval;
  int gaplen;
  unsigned long msecs = millis();
  unsigned long runmsec = millis() - _baseMillis;
  if( runmsec > 1000000 )
  {
    _baseMillis = msecs;
  }
  int ticks = runmsec/10;

  if( _lastTick != ticks )
  {
    _lastTick = ticks;
    
    if( sample() > 0 )
    {
      Serial.println( "noise detected");
      _noisy = true;
      _gapMillis = msecs;
    }

    if( _noisy && ((msecs - _gapMillis) > 500 ))
    {
      Serial.println("Closing Gap");
      _noisy = false;

    }
    if( _noisy && ticks % 5 == 0 )
    {
      shiftval = random(1,STRIPLEN/4);
      if(random(0,10) % 2 == 0)
        shiftval *= -1;
      shift(logicalStrip, shiftval );
    }
    if(  _noisy && ticks % 40 == 0 )
    {
      gaplen = random(3,7);
      gap( logicalStrip, gaplen );
      Serial.println(gaplen);
    }
    if ( _paramMode == MODE_CYCLE && ticks % _paramCycleSpeed == 0 )
    {
      initWheel(_offset);
      _offset = (_offset+=1)%256;
    }
  logical_show(logicalStrip);
  Blynk.run();
  }
}




