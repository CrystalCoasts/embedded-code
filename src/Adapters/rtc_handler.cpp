#include "rtc_handler.h"
#include "esp_sntp.h"
#include "NTPClient.h"
#include "globals.h"

Preferences preferences;


String RTC_TAG ="[RTC_TAG] ";
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
    String msg;
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        msg =  RTC_TAG + " Failed to obtain time";
        Serial.println(msg);
    } else {
        timeinfo.tm_year += 1900; // Adjust the year here
    }
    return timeinfo;
}

void on_time_sync(struct timeval *t) {
    String msg = RTC_TAG + "Time synchronized";
    Serial.println(msg);
    printLocalTime();
    time_synced = true;
    // we can update 'master esp32 here'
}


void printTime() {
    String msg;
    struct tm timeinfo = get_current_time();
    if (timeinfo.tm_year > (1970 - 1900)) { // Check if time is set
        msg =  RTC_TAG + " Current Time:";
        Serial.print(msg);
        Serial.printf(" %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    } else {
        msg =  RTC_TAG + " Time not set yet.";
        Serial.print(msg);
    }
}

uint8_t printLocalTime() {
    String msg;
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) {
        msg =  RTC_TAG + " Time not set yet.";
        Serial.print(msg);
        return 1;
    }
    char timeStr[80]; // Ensure the buffer is large enough to hold the resulting string
    strftime(timeStr, sizeof(timeStr), "%A, %B %d, %Y %H:%M:%S", &timeinfo);
    msg =  RTC_TAG + String(timeStr);
    Serial.println(msg);
    // Serial.println(&timeinfo, "%A, %B %d, %Y %H:%M:%S");
    return 0;
}

void updateSystemTime(const struct tm& newTime) {
    String msg;
    // Convert tm struct to time_t
    time_t t = mktime((struct tm *)&newTime);
    
    // Set system time
    timeval tv = { t, 0 };
    settimeofday(&tv, NULL); // Update the system time with new time

    // Logging for debugging
    msg = RTC_TAG + "System time updated successfully.";
    Serial.println(msg);
    // For further verification, you might want to print the new time
    printLocalTime();
}

void saveTimerSettings(uint64_t userPowerOn) {
    preferences.begin("my_timers", false); // Open NVS in read/write mode
    Serial.println("Saving power on timer: "+ String(userPowerOn));
    preferences.putInt("powerOnTimer", userPowerOn);
    preferences.end(); // Close NVS to save changes
}

void loadTimerSettings() {
    preferences.begin("my_timers", true); // Open NVS in read-only mode
    USER_POWER_ON = preferences.getInt("powerOnTimer", 5 * MINUTE_US); // default to 3 minutes if not set
    Serial.println("Loaded power on timer: "+ String(USER_POWER_ON));
    preferences.end(); // Close NVS after reading
}


