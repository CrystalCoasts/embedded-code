#include <Arduino.h>
#include "TempSensor.h"
#include "LcdDisplay.h"
#include <WiFi.h>
#include <HTTPClient.h>

#include "config.h"



const char* SSID = "Diane";
const char* PASSWD = "12345678";
const char* WEB_APP_URL = "https://script.google.com/macros/s/AKfycbyP09LCH7aRHxhd2tVU9Q3xyEZBOODTN3lYw031-epCypqzabFI7mX5h6Ue1ONw6iNY/exec";


void setup() {
  Serial.begin(115200);
  TempSensor::Get().begin();
  LcdDisplay::Get().init();
  WiFi.begin(SSID, PASSWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    LcdDisplay::Get().displayMessage("Connecting WIFI",0,0);
    Serial.println("Connecting ...");
    delay(1500);
    LcdDisplay::Get().clear();
  }
  LcdDisplay::Get().displayMessage("WiFi Connected",0,0);
  Serial.println("WiFi ✓");
  delay(1500);
  LcdDisplay::Get().clear();
}

void loop() {
  
   while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      LcdDisplay::Get().displayMessage("Connecting WIFI",0,0);
      Serial.println("Connecting ...");
      delay(1500);
      LcdDisplay::Get().clear();
  }
  float temperature = TempSensor::Get().readTemperature();
  float humidity = TempSensor::Get().readHumidity();
  String m1 = "Temp: " + String(temperature, 2) + " C";
  String m2 = "Humi: "+String(humidity)+ "%";
  LcdDisplay::Get().displayMessage(m1, 0, 0);
  LcdDisplay::Get().displayMessage(m2, 0, 1);

 

  if (WiFi.status()==WL_CONNECTED){
    HTTPClient http;
    http.begin(WEB_APP_URL);
    http.addHeader("Content-Type","application/json");

    String httpRequestData = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
    int httpResponseCode = http.POST(httpRequestData);
    // String payload = http.getString(); // Get the request response payload
    // Serial.println(payload);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
  delay(6000);
}
