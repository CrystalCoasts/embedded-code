#include "rtc_handler.h"
#include <WiFi.h>
#include "esp_sntp.h"
#include "NTPClient.h"
#include "WiFiUdp.h"

const char* NTP_SERVER_1 = "pool.ntp.org";
const char* NTP_SERVER_2 = "time.nist.gov";
const long GMT_OFFSET_SEC = -(3600 * 5);
const int DAYLIGHT_OFFSET_SEC = 3600;
const char* TIME_ZONE = "CET-1CEST,M3.5.0,M10.5.0/3";

bool time_synced = false;

void rtc_begin() {
    sntp_set_time_sync_notification_cb(on_time_sync);
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER_1, NTP_SERVER_2);
   
}

bool is_time_synced() {
    
    return time_synced;
}

struct tm get_current_time() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
    } else {
        timeinfo.tm_year += 1900; // Adjust the year here
    }
    return timeinfo;
}

void on_time_sync(struct timeval *t) {
    Serial.println("Time synchronized");
    printLocalTime();
    time_synced = true;
    // we can update 'master esp32 here'
}


void printTime() {
    struct tm timeinfo = get_current_time();
    if (timeinfo.tm_year > (1970 - 1900)) { // Check if time is set
        Serial.printf("Current Time: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    } else {
        Serial.println("Time not set yet.");
    }
}

uint8_t printLocalTime() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) {
        Serial.println("No time available (yet)");
        return 1;
    }
    Serial.println(&timeinfo, "%A, %B %d, %Y %H:%M:%S");
    return 0;
}

void updateSystemTime(const struct tm& newTime) {
    // Convert tm struct to time_t
    time_t t = mktime((struct tm *)&newTime);
    
    // Set system time
    timeval tv = { t, 0 };
    settimeofday(&tv, NULL); // Update the system time with new time

    // Logging for debugging
    Serial.println("System time updated successfully.");
    // For further verification, you might want to print the new time
    printLocalTime();
}