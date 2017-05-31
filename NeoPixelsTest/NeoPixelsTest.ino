
#include "FastLED.h"
#include <elapsedMillis.h>

#define LED_TYPE NEOPIXEL
#define LED_PIN_DATA 6
#define NUM_LEDS 100

#define BRIGHTNESS 128
  
uint8_t hue = 180;
uint8_t sat = 0;
uint8_t val = 100;

CRGB leds[NUM_LEDS];
elapsedMillis timerColor;

void setup() { 
  delay( 3000 ); // power-up safety delay
  
  FastLED.addLeds<LED_TYPE, LED_PIN_DATA>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  timerColor = 0;
  }
void loop() { 
  // blue to white (noon) to reddish brown
  FastLED.showColor(CHSV(hue, sat, val));
  delay(10);
  if(hue < 100) sat++;
  else sat--;
  if (sat == 0) hue = 30; 
  if (sat == 255) hue = 180;
}

void setIncrementFromCurrent(int *targetHSV, unsigned long targetTime, int *increments, unsigned long &timeIncrement)
{
  
}

void changeColorOverTime(int hueIncrement, int satIncrement, int valIncrement, unsigned long timeIncrement) 
{
  if (timerColor > timeIncrement) {
    timerColor -= timeIncrement;
    hue += hueIncrement;
    sat += satIncrement;
    val += valIncrement;
    FastLED.showColor(CHSV(hue, sat, val));
  }
}

