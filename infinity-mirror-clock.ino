#include <TinyWireM.h>
#include <USI_TWI_Master.h>
#include <TinyRTClib.h>
#include <Adafruit_NeoPixel.h>
const byte colorArray[60] ={83,101,119,136,153,169,184,198,211,222,232,240,247,251,254,255,254,251,247,240,232,222,211,198,184,169,153,136,119,101,83,65,47,30,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,30,47,65};

byte BRIGHTNESS_LEVEL = 15;

const byte BRIGHTNESS_LEVEL_MAX = 38;

const float RED_COEFF = 0.99;
const float GREEN_COEFF = 0.70;
const float BLUE_COEFF = 0.50;

const byte DELAY = 40;

RTC_DS1307 RTC;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, 4, NEO_GRB + NEO_KHZ800);

byte latch_left = 0;
byte latch_right = 0;
boolean latch_down = 0;
byte mode = 0;
byte brightness = 127;
byte red_factor = 255;
byte green_factor = 255;
byte blue_factor = 255;
boolean showBrightness = 0;
byte showBrightnessTimer = 0;

void calcBrightness()
{
  brightness = ((BRIGHTNESS_LEVEL*BRIGHTNESS_LEVEL_MAX*2.416)/60.0)+1;
  red_factor = brightness*RED_COEFF;
  green_factor = brightness*GREEN_COEFF;
  blue_factor = brightness*BLUE_COEFF;
}

void setup(){
  pinMode(1, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  calcBrightness();
  strip.begin();
  strip.setPixelColor(0, 255, 0, 0);
  strip.show();
  TinyWireM.begin();
  RTC.begin();
  DateTime now = RTC.now();
  RTC.adjust(now.unixtime() + 172800);
}

void loop(){
  DateTime now = RTC.now();
  
  byte sHour = byte((now.hour()%12)*5 + floor(now.minute()/12))%60;
  byte sMinute = now.minute();
  byte sSecond = now.second();
  
  for (byte i = 0; i < 60; i++) strip.setPixelColor(i, 0);
  
  for (byte i = 0; i < 60; i++)
  {
    switch(mode)
    {
      case 0: strip.setPixelColor(59-i, Wheel(((millis()/20)   +i) %60)); break;
      case 1: strip.setPixelColor(59-i, Wheel(((millis()/100)  +i) %60)); break;
      case 2: strip.setPixelColor(59-i, Wheel(((millis()/500)  +i) %60)); break;
      case 3: strip.setPixelColor(59-i, Wheel(((millis()/40)     ) %60)); break;
      case 4: strip.setPixelColor(59-i, Wheel(((millis()/200)    ) %60)); break;
      case 5: strip.setPixelColor(59-i, Wheel(((millis()/1000)   ) %60)); break;
      case 6: strip.setPixelColor(map(i, 0, 59, 0, 11)*5, Wheel(i));      break;
      case 7: strip.setPixelColor(map(i, 0, 59, 0, 11)*5, Wheel((millis() / 100) % 60)); break;
      case 8: strip.setPixelColor(map(i, 0, 59, 0, 11)*5, brightness, brightness, brightness);    break;
      case 9: strip.setPixelColor(59-i, 0); break;
      //case 10: strip.setPixelColor(i % byte(BRIGHTNESS_LEVEL), strip.Color(brightness, brightness, brightness)); break;
      default: mode = 0; break;
    }
  }
  
  if(mode < 6)
  {
    strip.setPixelColor((sHour+59)%60, 0);
    strip.setPixelColor((sHour+61)%60, 0);
    strip.setPixelColor((sMinute+59)%60, 0);
    strip.setPixelColor((sMinute+61)%60, 0);
    strip.setPixelColor((sSecond+59)%60, 0);
    strip.setPixelColor((sSecond+61)%60, 0);
  }
  
  if (mode < 10)
  {
    strip.setPixelColor(sMinute, 0);
    strip.setPixelColor(sSecond, 0);
    
    strip.setPixelColor(sHour, strip.Color(min(255,brightness*2), 0, 0));
    strip.setPixelColor(sMinute, strip.getPixelColor(sMinute) + strip.Color(0, min(255,brightness*2), 0));
    strip.setPixelColor(sSecond, strip.getPixelColor(sSecond) + strip.Color(0, 0, min(255,brightness*2)));
  }
  
  if (digitalRead(5) == LOW)
  {
    if (latch_down == 0) 
    {
      mode++;
    }
    latch_down=(latch_down+1)%(DELAY*5);
  }
  else {latch_down = 0;}
  
  if (digitalRead(1) == LOW)
  {
    if(mode != 9)
    {
      if (latch_left == 0) 
      {
        BRIGHTNESS_LEVEL = min(BRIGHTNESS_LEVEL++,60);
        calcBrightness();
      }
      showBrightness = 1;
      showBrightnessTimer = now.unixtime()%255;
      latch_left=(latch_left+4)%DELAY;
    }
    else
    {
      if (latch_left == 0) 
      {
        RTC.adjust(now.unixtime() + 60);
      }
      latch_left=(latch_left+1)%DELAY;
    }    
  }  
  else {latch_left = 0;}
  
  if (digitalRead(3) == LOW)
  {
    if(mode != 9)
    {
      if (latch_right == 0) 
      {
        BRIGHTNESS_LEVEL = max(BRIGHTNESS_LEVEL--,1);
        calcBrightness();
      }
      showBrightness = 1;
      showBrightnessTimer = now.unixtime()%255;
      latch_right=(latch_right+4)%DELAY;
    }
    else
    {
      if (latch_right == 0) 
      {
        RTC.adjust(now.unixtime() - 60);
      }
      latch_right=(latch_right+1)%DELAY;
    }    
  }  
  else {latch_right = 0;}
  
  if (showBrightness)
  {  
    if (showBrightnessTimer == now.unixtime()%255 || (showBrightnessTimer+1) == now.unixtime()%255)
      for(byte i = 0; i < BRIGHTNESS_LEVEL; i++) strip.setPixelColor(i, strip.Color(brightness, brightness, brightness));
    else
      showBrightness = 0;
  }
  
  strip.show();
}
  
uint32_t Wheel(byte x) {
  return strip.Color(map(colorArray[x],0,255,0,red_factor),map(colorArray[(x+20)%60],0,255,0,green_factor),map(colorArray[(x+40)%60],0,255,0,blue_factor));}

