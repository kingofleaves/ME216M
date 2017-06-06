#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Ticker.h>

//#define I2C 0x3C

#define WIFISSID "LOFT"
#define PASSWORD "loftloft"
//#define WIFISSID "ME216M Wi-Fi Network"
//#define PASSWORD "me216marduino"

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
  

void setup() {

  Serial.begin(115200);
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
  }
    httpSS.end();


}

void loop() {

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
