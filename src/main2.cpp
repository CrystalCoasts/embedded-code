#include <Arduino.h>
#include <Wire.h>
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include "SPI.h"
#include <HardwareSerial.h>
#include "FS.h"
#include "SdFat.h"
//#include <ESP32SPISlave.h>
#include "esp_sntp.h"
// #include "NTPClient.h"
#include "WiFiUdp.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32SPISlave.h>

byte mydata;
struct tm timeinfo;

const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = -(3600 * 5);
const int daylightOffset_sec = 3600;
const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";
const int yearOffset = 1900;

void receiver(int n_bytes)  {
    while(Wire.available())
        mydata = Wire.read();
}

void receiveEvent(int numBytes) {
  if (numBytes == sizeof(tm)) {
    Wire.readBytes((char *)&timeinfo, sizeof(tm));
    // Update system time
    timeval *t = {mktime(&timeinfo), 0};
    settimeofday(t, NULL);
    Serial.println("Time updated from master.");
  }
}

void printLocalTime()   {
    if(!getLocalTime(&timeinfo))    {
        Serial.println("No time available (yet)");
        return;
    }
    Serial.println(&timeinfo, "%A, %B %d, %Y %H:%M:%S");
}

// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t)
{
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}


void setup() {
    Serial.begin(115200);
    Wire.begin(0x10);
    Wire.onReceive(receiveEvent);

    sntp_set_time_sync_notification_cb(timeavailable);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
}

void loop() {
    delay(100);
    getLocalTime(&timeinfo);
    Serial.println(&timeinfo, "%A, %B %d, %Y %H:%M:%S");
}
