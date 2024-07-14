#ifndef RTC_HANDLER_H
#define RTC_HANDLER_H

#include <Arduino.h>
#include <time.h>


/* MACROS FOR TIME UNITS */

// Time units in milliseconds (for tasks)
#define SECOND_MS (1000)
#define MINUTE_MS (60 * SECOND_MS)
#define HOUR_MS (60 * MINUTE_MS)

// Time units in microseconds (for sleep modes)
#define SECOND_US (1000000)
#define MINUTE_US (60 * SECOND_US)
#define HOUR_US (60 * MINUTE_US)




// Function to initialize RTC and NTP settings
void rtc_begin();

// Function to get the current time
struct tm get_current_time();

// Callback function for time updates
void on_time_sync(struct timeval *t);

void printTime();

uint8_t printLocalTime();

void updateSystemTime(const struct tm& newTime);

#endif
