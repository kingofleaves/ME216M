#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Ticker.h>

#define I2C 0x3C

#define WIFISSID "LOFT"
#define PASSWORD "loftloft"

#define WORLDCLOCKAPIKEY "/api/json/est/now"
#define WCDOMAINNAME "worldclockapi.com"

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

// set time 
  ticker.attach(60, setReadyForTimeUpdate);

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
