#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Ticker.h>

#define I2C 0x3C

#define WIFISSID "kingofleaves"
#define PASSWORD "19940428"

#define SUNDOMAINNAME "api.sunrise-sunset.org"

// your network SSID (name)
char ssid[] = WIFISSID;

// your network password
char pass[] = PASSWORD;

// domain name
String domain = SUNDOMAINNAME;

//Latitude, Longitude
float latitude;
float longitude;
    
Ticker ticker;
bool readyForTimeUpdate = true;


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

// initialize latitude and longitude
  latitude = 0.12;
  longitude = 0.12;

// set time 
  ticker.attach(60, setReadyForTimeUpdate);
}

void loop() {

if (readyForTimeUpdate) {
  readyForTimeUpdate = false;
  
  HTTPClient http;
  String key = "";
  key = key + "/json?" + "lat=" + (String)latitude + "&" + "lng=" + (String)longitude;
  http.begin(domain, 80, key);
  int httpCode = http.GET();
  //Serial.println(httpCode);
  if (httpCode == HTTP_CODE_OK) { 
    String Stringpayload = http.getString();
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

    http.end();
}

}

void setReadyForTimeUpdate(void) {
  readyForTimeUpdate = true;
}

void printCurrLatLong(void) {
  Serial.println("Our current Latitude and Longitude is: ");
  Serial.print("\t");
  Serial.print(abs(latitude));
  Serial.print(latitude < 0 ? "S" : "N");
  Serial.print(", ");
  Serial.print(abs(longitude));
  Serial.print(longitude < 0 ? "W" : "E");
  Serial.println();
}

