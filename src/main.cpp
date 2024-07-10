#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include "FS.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>


#include "websockets.h"
const char* SSID = "eero-wlan";
const char* PASSWD = "Theknight17";
// const char* WEB_APP_URL = "https://script.google.com/macros/s/AKfycbzdREDYLRb1ew8CjwGY_WnrIU0UWW0Sn3Wr4XdT8Jv0VjXuQxJV7GVCKZeYtEb2zrKb/exec";



void setup() {
    Serial.begin(115200);
    WiFi.begin(SSID, PASSWD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    ws.init();
}



void loop() {
    ws.begin();
    
    // delay(1000);
    
    if(millis() % 30000 < 50) {  // Every 30 seconds, send a message
        ws.send_to_ws("Ping from ESP32");
    }
}

