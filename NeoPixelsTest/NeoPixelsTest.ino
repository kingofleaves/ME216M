#include <FastLED.h>

#define LED_TYPE NEOPIXEL
#define LED_PIN_DATA 6
#define NUM_LEDS 150

#define INITIAL_BRIGHTNESS 64

#define TIME_TO_EXTEND 1000 // in milliseconds
  
uint8_t hue = 180;
uint8_t sat = 0;
uint8_t val = 100;
uint8_t brightness;
uint8_t blendSpeed = 24;

unsigned long currTime;

// States
bool state_ON;
bool state_DISABLE;
bool state_EXTEND;

// variables for each mode
float disableTimeEnd;
float extendTimeEnd;


// Debug States
bool onoff = false;
bool disable = false;
bool extend = false;

// LED Defs
CRGB leds[NUM_LEDS];

// Color Defs
CRGB x = CRGB::Black;
CRGB w = CRGB::White;
CRGB o = CHSV( HUE_ORANGE, 255, 255);
CRGB b = CRGB::Blue;

// Color Palettes
CRGBPalette16 offPalette = CRGBPalette16( x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x );
CRGBPalette16 whitePalette = CRGBPalette16( w,w,w,w, w,w,w,w, w,w,w,w, w,w,w,w );
CRGBPalette16 orangePalette = CRGBPalette16( o,o,o,o, o,o,o,o, o,o,o,o, o,o,o,o );
CRGBPalette16 bluePalette = CRGBPalette16( b,b,b,b, b,b,b,b, b,b,b,b, b,b,b,b );

CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;


void setup() { 
  Serial.begin(115200);
  delay( 3000 ); // power-up safety delay

  FastLED.addLeds<LED_TYPE, LED_PIN_DATA>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  brightness = INITIAL_BRIGHTNESS;  
  FastLED.setBrightness( brightness );

  currentPalette = offPalette; // color of LEDs are currently all black (off).
  targetPalette = offPalette; // color of LEDs are approaching all black (off).

  state_ON = false; // lamp is currently off.
  state_DISABLE = false; // disable mode is not active.
  state_EXTEND = false; // extend mode is not active.

  Serial.println("Setup Complete!");
  currTime = 0;
}

void loop() { 
  checkForInputs();
  handleComputation();
  //debug();
}

void debug(void)
{
  if(state_ON) Serial.println("STATE ON");
  if(state_DISABLE) Serial.println("STATE DISABLE");
  if(state_EXTEND) Serial.println("STATE EXTEND");
}

void checkForInputs(void)
{
  if (onoffButtonPressed()) respond_to_onoffButton();
  if (disableButtonPressed()) respond_to_disableButton();
  if (extendButtonPressed()) respond_to_extendButton();
  updateBrightnessFromInput();
}

void handleComputation(void)
{
  // NOT IMPLEMENTED YET
  // check for states and do appropriate stuff
  // set target palettes and change color based on time.
  
  
  static uint8_t startIndex = 0;

  if ((millis() - currTime) > 10) {
    changeColorOverTime();  
    FillLEDsFromPaletteColors(startIndex++);
    FastLED.show();

    currTime += 10;
  }
}

//////////////////////////////////////////
//                                      //
//         ON/OFF BUTTON START          //
//                                      //
//////////////////////////////////////////

bool onoffButtonPressed(void)
{
  // NOT IMPLEMENTED YET
  if (onoff) {
    onoff = false;
    return true;
  }
  return false;
}

void respond_to_onoffButton(void)
{
  if (state_ON) {
    targetPalette = offPalette;
  }
  else {
    setPaletteFromTime();
  }
  state_ON = !state_ON;
}

//////////////////////////////////////////
//                                      //
//          ON/OFF BUTTON END           //
//                                      //
//////////////////////////////////////////

//////////////////////////////////////////
//                                      //
//         DISABLE BUTTON START         //
//                                      //
//////////////////////////////////////////


bool disableButtonPressed(void)
{
  // NOT IMPLEMENTED YET
  if (disable) {
    disable = false;
    return true;
  }
  return false;
}

void respond_to_disableButton(void)
{
  state_EXTEND = true;
  extendTimeEnd = millis() + TIME_TO_EXTEND;
}


//////////////////////////////////////////
//                                      //
//          DISABLE BUTTON END          //
//                                      //
//////////////////////////////////////////

//////////////////////////////////////////
//                                      //
//         EXTEND BUTTON START          //
//                                      //
//////////////////////////////////////////


bool extendButtonPressed(void)
{
  // NOT IMPLEMENTED YET
  if (extend) {
    extend = false;
    return true;
  }
  return false;
}

void respond_to_extendButton(void)
{
  state_EXTEND = true;
  extendTimeEnd = millis() + TIME_TO_EXTEND;
}

//////////////////////////////////////////
//                                      //
//          EXTEND BUTTON END           //
//                                      //
//////////////////////////////////////////

//////////////////////////////////////////
//                                      //
//       UPDATE BRIGHTNESS START        //
//                                      //
//////////////////////////////////////////

void updateBrightnessFromInput(void)
{
    uint8_t newBrightness = getBrightnessValue();
    if (abs(newBrightness - brightness) > 10) {
      brightness = newBrightness;
      FastLED.setBrightness( brightness );
    }
}

//////////////////////////////////////////
//                                      //
//        UPDATE BRIGHTNESS END         //
//                                      //
//////////////////////////////////////////

void setBlendSpeed(uint8_t newSpeed)
{
  blendSpeed = newSpeed;
}

void changeColorOverTime() 
{
  // blendSpeed ranges from 0 to 48, with 48 being the fastest, 1 being the slowest, and 0 meaning no change.
  nblendPaletteTowardPalette(currentPalette, targetPalette, blendSpeed);
}

void setPaletteFromTime(void)
{
  // NOT IMPLEMENTED YET
  targetPalette = whitePalette;
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{  
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex + sin8(i*16), brightness);
    colorIndex += 3;
  }
}

uint8_t getBrightnessValue(void)
{
  //NOT IMPLEMENTED YET
  return INITIAL_BRIGHTNESS;
}

//////////////////////////////////////////
//                                      //
//         DEBUGGING FUNCTIONS          //
//                                      //
//////////////////////////////////////////

void switchToDaySettings(void)
{
  targetPalette = whitePalette;
}

void switchToNightSettings(void)
{
  targetPalette = orangePalette;
}

// Debugging with Serial
void serialEvent() {

  char c = (char)Serial.read();

  switch (c)
  {
    case 'o': 
      onoff = true; 
      break;
    case 'd': 
      disable = true;
      break;
    case 'm':
      switchToNightSettings();
      break;
    case 's':
      switchToDaySettings();
      break;
    case '+':
      blendSpeed++;
      break;
    case '-':
      blendSpeed--;
      break;
      
    default:
      Serial.println("Not a valid command.");
      break;
  }
}

