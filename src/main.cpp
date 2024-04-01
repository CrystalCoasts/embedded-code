#include <Arduino.h>
#include "TempSensor.h"
#include "LcdDisplay.h"
#include <WiFi.h>
#include <HTTPClient.h>

#include "config.h"

// const char* ssid = "medievaltimes24";
// const char* password = "Theknight17";
// const char* webAppUrl = "https://script.google.com/macros/s/AKfycbzYB2tJTz9TIIO9vl1gc1nN6zH9-gDVDOI8uOUWh_cr3ZKJ1NBX-CI4uqTXxFpiLNZ8/exec";

// int mem = LCD_ADDRESS;

const char* mySSID = "networkName";
const char* myPASSWD = "networkPassword";
const char* myWEB_APP_URL = "https://script.google.com/macros/s/AKfycbzYB2tJTz9TIIO9vl1gc1nN6zH9-gDVDOI8uOUWh_cr3ZKJ1NBX-CI4uqTXxFpiLNZ8/exec";


void setup() {
  Serial.begin(115200);
  TempSensor::Get().begin();
  LcdDisplay::Get().init();
  WiFi.begin(mySSID, myPASSWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    LcdDisplay::Get().displayMessage("Connecting WIFI",0,0);
    Serial.println("Connecting ...");
  }
  LcdDisplay::Get().displayMessage("WiFi Connected",0,0);
  Serial.println("WiFi ✓");
  delay(1000);
  LcdDisplay::Get().clear();
}

void loop() {
  
  
  float temperature = TempSensor::Get().readTemperature();
  float humidity = TempSensor::Get().readHumidity();
  String m1 = "Temp: " + String(temperature, 2) + " C";
  String m2 = "Humi: "+String(humidity)+ "%";
  LcdDisplay::Get().displayMessage(m1, 0, 0);
  LcdDisplay::Get().displayMessage(m2, 0, 1);

  if (WiFi.status()==WL_CONNECTED){
    HTTPClient http;
    http.begin(myWEB_APP_URL);
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
