#ifndef RTC_HANDLER_H
#define RTC_HANDLER_H

#include <Arduino.h>
#include <time.h>





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
