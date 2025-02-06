#include "rtc_handler.h"
#include "esp_sntp.h"
#include "NTPClient.h"
#include "globals.h"

// Preferences preferences;

time_t localnow = time(NULL);
bool overflow = false;

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

    setenv("TZ", "America/New_York", 1);  // Replace with your timezone
    tzset();
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

bool getCurrentTime(tm timeinfo) {        //Cellular
    if(is_time_synced())   {
        getLocalTime(&timeinfo, 500);
    }else{
        if(!sim.isConnected())
            return false;
        std::string clk = sim.sendData("AT+CCLK?");
        
        // Extract the quoted time string
        int start = clk.find('"');  // First double quote
        int end = clk.find_last_of('"');  // Last double quote
        if (start == std::string::npos || end == std::string::npos) {
            Serial.println("Error: Could not find time string in response!");
            //return timeinfo; // Return empty struct
            return false;
        }
        clk = clk.substr(start + 1, end - start - 1);  // Extract time string

        // Parse date and time
        char delimiter = '/';
        int year = 2000 + std::stoi(clk.substr(0, clk.find(delimiter)));  // Convert year properly
        clk.erase(0, clk.find(delimiter) + 1);
        int month = std::stoi(clk.substr(0, clk.find(delimiter)));
        clk.erase(0, clk.find(delimiter) + 1);
        
        delimiter = ',';
        int day = std::stoi(clk.substr(0, clk.find(delimiter)));
        clk.erase(0, clk.find(delimiter) + 1);
        
        delimiter = ':';
        int hour = std::stoi(clk.substr(0, clk.find(delimiter)));
        clk.erase(0, clk.find(delimiter) + 1);
        int minute = std::stoi(clk.substr(0, clk.find(delimiter)));
        clk.erase(0, clk.find(delimiter) + 1);

        delimiter = '-';
        int second = std::stoi(clk.substr(0, clk.find(delimiter)));
        
        // Populate struct tm
        timeinfo.tm_year = year - 1900;  // Years since 1900
        timeinfo.tm_mon = month - 1;  // Months are 0-based
        timeinfo.tm_mday = day;
        timeinfo.tm_hour = hour;
        timeinfo.tm_min = minute;
        timeinfo.tm_sec = second;

        Serial.printf("Parsed Time: %04d/%02d/%02d %02d:%02d:%02d\n", 
                    year, month, day, hour, minute, second);
        updateSystemTime(timeinfo);        
        time_synced = true;
        return true;
    }
}

void on_time_sync(struct timeval *t) {
    String msg = RTC_TAG + "Time synchronized";
    Serial.println(msg);
    printLocalTime();
    time_synced = true;
    // we can update 'master esp32 here'
}


void printTime(struct tm* timeinfo) {
    String msg;
    //struct tm timeinfo;
    #ifndef CELLULAR
        struct tm timeinfo = get_current_time();
    #else
        // getCurrentTime(&timeinfo);
        // updateSystemTime(timeinfo);
        // timeinfo = get_current_time();
    #endif
    if (timeinfo->tm_year > (1970 - 1900)) { // Check if time is set
        msg =  RTC_TAG + " Current Time:";
        Serial.print(msg);
        Serial.printf(" %02d:%02d:%02d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    } else {
        msg =  RTC_TAG + " Time not set yet.";
        Serial.print(msg);
    }
}

uint8_t printLocalTime() {
    String msg;
    struct tm timeinfo;

    // if(!getLocalTime(&timeinfo)) {
    //     msg =  RTC_TAG + " Time not set yet.";
    //     Serial.print(msg);
    // }
    //updateSystemTime(timeinfo);

    char timeStr[80]; // Ensure the buffer is large enough to hold the resulting string
    strftime(timeStr, sizeof(timeStr), "%A, %B %d, %Y %H:%M:%S", &timeinfo);
    msg =  RTC_TAG + String(timeStr);
    Serial.println(msg);
    Serial.println(&timeinfo, "%A, %B %d, %Y %H:%M:%S");
    return 0;
}

void updateSystemTime(tm newTime) {
    String msg;
    
    printTime(&newTime);
    // Convert tm struct to time_t
    time_t t = mktime(&newTime);
    if (t == -1) {
        Serial.println("Error: Failed to convert struct tm to time_t!");
        return;
    }
    // Set system time
    struct timeval tv;
    if (t > 2082758399){
        overflow = true;
        tv.tv_sec = t - 2082758399;  // epoch time (seconds)
    } else {
        overflow = false;
        tv.tv_sec = t;  // epoch time (seconds)
    }
    //tv.tv_usec = 0;    // microseconds

    if(settimeofday(&tv,NULL) != 0 ) {
        Serial.println("Error: Failed to set system time!");
    }
    configTime(0, 0, "");  // Clear NTP sync

    // Update the system time with new time

    // Logging for debugging
    msg = RTC_TAG + "System time updated successfully.";
    Serial.println(msg);
    // For further verification, you might want to print the new time
    struct tm verifiedTime = get_current_time();
    if (getLocalTime(&verifiedTime)) {  // Ensure it gets the correct time
        Serial.printf("[RTC_TAG] Verified System Time: %04d-%02d-%02d %02d:%02d:%02d\n",
                      verifiedTime.tm_year + 1900, verifiedTime.tm_mon + 1, verifiedTime.tm_mday,
                      verifiedTime.tm_hour, verifiedTime.tm_min, verifiedTime.tm_sec);
    } else {
        Serial.println("[RTC_TAG] Failed to retrieve updated time!");
    }
    //getLocalTime(&verifiedTime);
    //printTime(&verifiedTime);
    //printLocalTime();
}

void saveTimerSettings(uint64_t userPowerOn) {
    prefs.begin("my_timers", false); // Open NVS in read/write mode
    Serial.println("Saving user_power on timer: "+ String(userPowerOn));
    prefs.putInt("powerOnTimer", userPowerOn);
    prefs.end(); // Close NVS to save changes
}

void loadTimerSettings() {
    prefs.begin("my_timers", true); // Open NVS in read-only mode
    USER_POWER_ON = prefs.getInt("powerOnTimer", 5 * MINUTE_US); // default to 3 minutes if not set
    Serial.println("Loaded User power_on timer: "+ String(USER_POWER_ON));
    prefs.end(); // Close NVS after reading
}


