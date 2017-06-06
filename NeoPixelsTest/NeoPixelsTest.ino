#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include "Wire.h"

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Ticker.h>

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

#define POT_PIN A0
#define SNOOZE_PIN 13

uint8_t buffer_m[6];

int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;

float Axyz[3];

#define sample_num_mdate  5000      



// API Defs
#define WIFISSID "ME216M Wi-Fi Network"
#define PASSWORD "me216marduino"

#define WORLDCLOCKAPIKEY "/api/json/est/now"
#define WCDOMAINNAME "worldclockapi.com"

#define GEOLOCAPIKEY "/json"
#define GLDOMAINNAME "ip-api.com"

#define SUNDOMAINNAME "api.sunrise-sunset.org"

// your network SSID (name)
char ssid[] = WIFISSID;

// your network password
char pass[] = PASSWORD;

// Go to forecast.io and register for an API KEY
String worldClockApiKey = WORLDCLOCKAPIKEY;

// website domain name
String webDomain = WCDOMAINNAME;
    
Ticker ticker;
bool readyForTimeUpdate = true;

String lon = "0";
String lat = "0";
  




// LED Defs
#define LED_TYPE NEOPIXEL
#define LED_PIN_DATA 2
#define NUM_LEDS 60

#define INITIAL_BRIGHTNESS 128

#define TIME_TO_EXTEND 10000 // in milliseconds
#define TIME_TO_DISABLE 10000 // in milliseconds

#define BLEND_INTERVAL 10 // in milliseconds
#define INITIAL_BLEND 36

uint8_t hue = 180;
uint8_t sat = 0;
uint8_t val = 100;
uint8_t brightness;
uint8_t blendSpeed = 24;

int potVal = 0;

unsigned long currTime;

// States
bool state_ON;
bool state_DISABLE = false;
bool state_EXTEND;

int state_PREV_button = LOW;
int state_CURR_button = LOW;

long time1 = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers


// variables for each mode
unsigned long disableTimeEnd;
unsigned long extendTimeEnd;

long timeNow = 0;

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
CRGB r = CHSV( HUE_RED, 255, 128);

// Color Palettes
CRGBPalette16 offPalette = CRGBPalette16( x );
CRGBPalette16 whitePalette = CRGBPalette16( w );
CRGBPalette16 orangePalette = CRGBPalette16( o );
CRGBPalette16 bluePalette = CRGBPalette16( b );
CRGBPalette16 redPalette = CRGBPalette16( r );
CRGBPalette16 sunrisePalette = whitePalette;
CRGBPalette16 sunsetPalette = orangePalette;

static long const totalDuration = 60000;
static long const timeBlue = 0;
static long const timeWhite = 10000;
static long const timeOrange = 20000;
static long const timeRed = 40000;
static long const timeRed2 = 55000;


CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;

// API stuff
long timeSunrise;
long timeSunset;


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

// API Setup
  WiFi.mode(WIFI_AP);

  WiFi.begin(ssid, pass);

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter++;
  }


  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

// get Lat and Lon from IP address API
  HTTPClient httpGL;
  httpGL.begin(GLDOMAINNAME, 80, GEOLOCAPIKEY);
  int httpGLCode = httpGL.GET();
  //Serial.println(httpCode);


  if (httpGLCode == HTTP_CODE_OK) {
    String Stringpayload = httpGL.getString();
    DynamicJsonBuffer jsonBuffer(400);
    JsonObject& root = jsonBuffer.parseObject(Stringpayload, 10);
    String city = root["city"];
    String state = root["regionName"];
    String lat1 = root["lat"];
    String lon1 = root["lon"];
    lat = lat1;
    lon = lon1;
    Serial.print(city);
    Serial.println(state);
    Serial.println(lat);
    Serial.println(lon);
  }
    httpGL.end();

// get Sunrise and Sunset data using Lat and Lon from Geolocation API
  HTTPClient httpSS;
  String key = "";
  key = key + "/json?" + "lat=" + lat + "&" + "lng=" + lon;
  httpSS.begin(SUNDOMAINNAME, 80, key);
  int httpSSCode = httpSS.GET();
  //Serial.println(httpCode);
  if (httpSSCode == HTTP_CODE_OK) { 
    String Stringpayload = httpSS.getString();
    DynamicJsonBuffer jsonBuffer(400);
    JsonObject& root = jsonBuffer.parseObject(Stringpayload, 10);
    JsonObject& results = root["results"];
    String sunrise = results["sunrise"];
    String sunset = results["sunset"];
    String civil_twilight_begin = results["civil_twilight_begin"];
    String civil_twilight_end = results["civil_twilight_end"];

    printCurrLatLong();
    
    Serial.println(sunrise + " is sunrise.");
    Serial.println(sunset + " is sunset.");
    Serial.println(civil_twilight_begin + " is start of civil twilight.");
    Serial.println(civil_twilight_end + " is end of civil twilight.");

    //TODO: Parse Sunset and Surise into time to be used by this code.
  }
    httpSS.end();



  Serial.println("Setup Complete!");
  currTime = 0;

  pinMode(POT_PIN,INPUT);
  pinMode(SNOOZE_PIN,INPUT);
    
}

void loop() {
  currTime = millis();
  getTimeFromAPI();
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

  setTargetPalette();
  
  checkDisable();
  checkExtend();

  changeColorOverTime();

}


//////////////////////////////////////////
//                                      //
//        API CONTROL FUNCTIONS         //
//                                      //
//////////////////////////////////////////


void getTimeFromAPI(void) {
  
  if (readyForTimeUpdate) {
    readyForTimeUpdate = false;
    
    HTTPClient http;
    http.begin(WCDOMAINNAME, 80, WORLDCLOCKAPIKEY);
    int httpCode = http.GET();
    //Serial.println(httpCode);
    if (httpCode == HTTP_CODE_OK) {
      String Stringpayload = http.getString();
      DynamicJsonBuffer jsonBuffer(400);
      JsonObject& root = jsonBuffer.parseObject(Stringpayload, 10);
      String dateTime = root["currentDateTime"];
      String zone = root["timeZoneName"];
      Serial.println(dateTime);
      Serial.println(zone);
    }

    http.end();
  }
}

void setReadyForTimeUpdate() {
  readyForTimeUpdate = true;
}

void printCurrLatLong(void) {
  Serial.println("Our current Latitude and Longitude is: ");
  Serial.print("\t");
  Serial.print(abs(lat.toFloat()));
  Serial.print(lat.toFloat() < 0 ? "S" : "N");
  Serial.print(", ");
  Serial.print(abs(lon.toFloat()));
  Serial.print(lon.toFloat() < 0 ? "W" : "E");
  Serial.println();
}

//////////////////////////////////////////
//                                      //
//         ON/OFF BUTTON START          //
//                                      //
//////////////////////////////////////////

bool onoffButtonPressed(void)
{

  getAccel_Data();
//  Serial.print(Axyz[0]);
//  Serial.print(Axyz[1]);
//  Serial.println(Axyz[2]);
  if (abs(Axyz[1]) > 0.5) {
    // Y reading is too high -> too tilted
//    Serial.println("OFF");
    return false;
  }
  if (abs(Axyz[2]) > abs(2*Axyz[0]) + 0.1) {
    // Z > 2X -> turn off
//    Serial.println("OFF");
    return false;
  }
  if (abs(Axyz[0]) > abs(2*Axyz[2]) + 0.1) {
    // X > 2Z -> turn on
//    Serial.println("ON");
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
  state_CURR_button = digitalRead(SNOOZE_PIN);
  if (state_CURR_button == HIGH && state_PREV_button == LOW && millis() - time1 > debounce) {
    Serial.println("button pressed!");
    time1 = millis();
    state_PREV_button = state_CURR_button;
    return true;    
  } else {
    state_PREV_button = state_CURR_button;
    return false;
  }
}

void respond_to_disableButton(void)
{
  // state_DISABLE = true; For longer snooze
  state_DISABLE = !state_DISABLE; // For unsnoozing at accidental snooze
  disableTimeEnd = currTime + TIME_TO_DISABLE;
  timeNow = timeNow - 20000;
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
  //if (abs(newBrightness - brightness) > 10) {
    brightness = newBrightness;
    FastLED.setBrightness( brightness );
  //}
}

uint8_t getBrightnessValue(void)
{
  potVal = analogRead(POT_PIN);
  //Serial.println(potVal);
  int mappedPotVal = map(potVal,0,1023,0,255);
  return mappedPotVal;
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
  if (millis() < lastBlend) lastBlend = millis(); // So this can run over the capacity of millis i.e. signed long
  if ((millis() - lastBlend) > BLEND_INTERVAL) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, blendSpeed);
    FillLEDsFromPaletteColors(startIndex++);
    lastBlend += BLEND_INTERVAL;
  }
}

void setTargetPalette(void){
  // NOT IMPLEMENTED YET
  if (!state_ON) targetPalette = offPalette;
  else if (state_DISABLE) targetPalette = whitePalette;
  else {
    setPaletteFromTime(); // sets targetPalette to a blend based on time.
  }
}

CRGBPalette16 setPaletteFromTime(void)
{
  // targetPalette = orangePalette;
// WORK IN PROGRESS //
  //if(state_DISABLE == false) {
  timeNow = getTimeFromAPI(totalDuration);
  static long timeStart;
  static long timeEnd;
  CRGBPalette16 startPalette;
  CRGBPalette16 endPalette;

  if (timeNow < timeWhite) {
    blendSpeed = INITIAL_BLEND;
    timeStart = timeBlue;
    timeEnd = timeWhite;
    startPalette = bluePalette;
    endPalette = whitePalette;
  }
  else if (timeNow < timeOrange) {
    blendSpeed = INITIAL_BLEND;
    timeStart = timeWhite;
    timeEnd = timeOrange; 
    startPalette = whitePalette;
    endPalette = orangePalette;
  }
  else if (timeNow < timeRed) {
    blendSpeed = INITIAL_BLEND/3;
    timeStart = timeOrange;
    timeEnd = timeRed;
    startPalette = orangePalette;
    endPalette = redPalette;
  }  
  else if (timeNow < timeRed2) {
    blendSpeed = INITIAL_BLEND/3;
    timeStart = timeRed;
    timeEnd = timeRed2;
    startPalette = redPalette;
    endPalette = redPalette;
  }
  else {
    blendSpeed = INITIAL_BLEND/3;
    timeStart = timeRed2;
    timeEnd = totalDuration;
    startPalette = redPalette;
    endPalette = bluePalette;
  }
  
  float rawBlendRatio = ((float)(timeNow - timeStart))/(timeEnd - timeStart);
  uint8_t blendRatio = rawBlendRatio * 256;
  Serial.println(blendRatio);
  uint8_t paletteSize = sizeof( targetPalette) / sizeof(targetPalette[0]); // = 16
  return blend(startPalette, endPalette, currentPalette, paletteSize, blendRatio);
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
  for ( int i = 0; i < NUM_LEDS; i++) {
//    // TEST
//    uint8_t flickeringBrightness = (float)brightness/256 * sin8(i + millis()%256);
//    leds[i] = ColorFromPalette( currentPalette, colorIndex + sin8(i * 16), flickeringBrightness);
//    // TEST END
    leds[i] = ColorFromPalette( currentPalette, colorIndex + sin8(i * 16), brightness);
    colorIndex += 3;
  }
}

//////////////////////////////////////////
//                                      //
//            API FUNCTIONS             //
//                                      //
//////////////////////////////////////////


long getTimeFromAPI(long modDuration)
{
  //NOT IMPLEMENTED YET
  return (millis()%modDuration);
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

