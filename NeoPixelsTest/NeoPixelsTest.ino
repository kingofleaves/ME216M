#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include "Wire.h"


// I2Cdev and MPU9250 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU9250.h"

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU9250 accelgyro;
I2Cdev   I2C_M;

// I2C Pin Designations
#define SDA 4
#define SCL 5

uint8_t buffer_m[6];

int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;

float Axyz[3];

#define sample_num_mdate  5000      



// LED Defs
#define LED_TYPE NEOPIXEL
#define LED_PIN_DATA 2
#define NUM_LEDS 60

#define INITIAL_BRIGHTNESS 128

#define TIME_TO_EXTEND 10000 // in milliseconds
#define TIME_TO_DISABLE 10000 // in milliseconds

#define BLEND_INTERVAL 10 // in milliseconds

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
unsigned long disableTimeEnd;
unsigned long extendTimeEnd;


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
CRGBPalette16 offPalette = CRGBPalette16( x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x );
CRGBPalette16 whitePalette = CRGBPalette16( w, w, w, w, w, w, w, w, w, w, w, w, w, w, w, w );
CRGBPalette16 orangePalette = CRGBPalette16( o, o, o, o, o, o, o, o, o, o, o, o, o, o, o, o );
CRGBPalette16 bluePalette = CRGBPalette16( b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b );

CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;


void setup() {
  Serial.begin(115200);
  delay( 3000 ); // power-up safety delay

  // IMU Setup:
    // join I2C bus (I2Cdev library doesn't do this automatically)
  Wire.begin(SDA, SCL);
    // initialize device
  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();
  Serial.println(accelgyro.testConnection() ? "MPU9250 connection successful" : "MPU9250 connection failed");

  
  // LED Setup:
  FastLED.addLeds<LED_TYPE, LED_PIN_DATA>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  brightness = INITIAL_BRIGHTNESS;
  FastLED.setBrightness( brightness );

  currentPalette = whitePalette; // color of LEDs start with white.
  targetPalette = offPalette; // color of LEDs are approaching all black (off).

  state_ON = false; // lamp is currently off.
  state_DISABLE = false; // disable mode is not active.
  state_EXTEND = false; // extend mode is not active.

  Serial.println("Setup Complete!");
  currTime = 0;
}

void loop() {
  currTime = millis();
  checkForInputs();
  serialEvent();
  handleComputation();
  FastLED.show();
  debug();
}

void debug(void)
{
  if (state_ON) Serial.println("STATE ON");
  if (state_DISABLE) Serial.println("STATE DISABLE");
  if (state_EXTEND) Serial.println("STATE EXTEND");
}

void checkForInputs(void)
{
  respond_to_onoffButton();
  if (disableButtonPressed()) respond_to_disableButton();
  if (extendButtonPressed()) respond_to_extendButton();
  updateBrightnessFromInput();
}

void handleComputation(void)
{
  // NOT IMPLEMENTED YET
  // check for states and do appropriate stuff
  // set target palettes and change color based on time.

  setPaletteFromTime();

  checkDisable();
  checkExtend();

  changeColorOverTime();

}

//////////////////////////////////////////
//                                      //
//         ON/OFF BUTTON START          //
//                                      //
//////////////////////////////////////////

bool onoffButtonPressed(void)
{

  getAccel_Data();
  Serial.print(Axyz[0]);
  Serial.print(Axyz[1]);
  Serial.println(Axyz[2]);
  if (abs(Axyz[1]) > 0.5) {
    // Y reading is too high -> too tilted
    Serial.println("OFF");
    return false;
  }
  if (abs(Axyz[2]) > abs(2*Axyz[0]) + 0.1) {
    // Z > 2X -> turn off
    Serial.println("OFF");
    return false;
  }
  if (abs(Axyz[0]) > abs(2*Axyz[2]) + 0.1) {
    // X > 2Z -> turn on
    Serial.println("ON");
    return true;
  }
  return false;
  
//  // NOT IMPLEMENTED YET
//  if (onoff) {
//    onoff = false;
//    return true;
//  }
//  return false;
}

void respond_to_onoffButton(void)
{
  state_ON = onoffButtonPressed();
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
  state_DISABLE = true;
  disableTimeEnd = currTime + TIME_TO_DISABLE;
}

void checkDisable(void)
{
  if (state_DISABLE) {
    if (currTime > disableTimeEnd) {
      state_DISABLE = false;
    }
  }
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
  extendTimeEnd = currTime + TIME_TO_EXTEND;
}

void checkExtend(void)
{
  if (state_EXTEND) {
    if (currTime > extendTimeEnd) {
      state_EXTEND = false;
    }
  }
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

void changeColorOverTime(void)
{
  // blendSpeed ranges from 0 to 48, with 48 being the fastest, 1 being the slowest, and 0 meaning no change.
  static uint8_t startIndex = 0;
  static long lastBlend = millis();
  if ((millis() - lastBlend) > BLEND_INTERVAL) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, blendSpeed);
    FillLEDsFromPaletteColors(startIndex++);
    lastBlend += BLEND_INTERVAL;
  }
}

void setPaletteFromTime(void)
{
  // NOT IMPLEMENTED YET
  if (!state_ON) targetPalette = offPalette;
  else if (state_DISABLE) targetPalette = whitePalette;
  else targetPalette = orangePalette;

}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex + sin8(i * 16), brightness);
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
//            IMU FUNCTIONS             //
//                                      //
//////////////////////////////////////////

void getAccel_Data(void)
{
  accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
  Axyz[0] = (double) ax / 16384;//16384  LSB/g
  Axyz[1] = (double) ay / 16384;
  Axyz[2] = (double) az / 16384; 
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

  if (Serial.available() > 0) {
    char c = (char)Serial.read();
    Serial.print("Read the character: ");
    Serial.println(c);
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
}

