#ifndef RTC_HANDLER_H
#define RTC_HANDLER_H

#include <Arduino.h>
#include <time.h>
#include "Cellular.h"


/* MACROS FOR TIME UNITS */

// Time units in milliseconds (for tasks)
#define SECOND_MS (1000)
#define MINUTE_MS (60 * SECOND_MS)
#define HOUR_MS (60 * MINUTE_MS)

// Time units in microseconds (for sleep modes)
#define SECOND_US (1000000ULL)
#define MINUTE_US (60ULL * SECOND_US)
#define HOUR_US (60ULL * MINUTE_US)

#define CELLULAR

//time_t local;

// Function to initialize RTC and NTP settings
void rtc_begin();

// Function to get the current time
struct tm get_current_time();
bool getCurrentTime(struct tm* timeinfo);     //updated too support cellular instead of wifi
void updateSystemTime(struct tm* newTime);

// Callback function for time updates
void on_time_sync(struct timeval *t);

void printTime(struct tm* timeinfo);

uint8_t printLocalTime();

//void updateSystemTime(tm& newTime);
void saveTimerSettings(uint64_t userPowerOn);
void loadTimerSettings();

#endif
