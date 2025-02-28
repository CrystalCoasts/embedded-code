#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>
#include "Preferences.h"

// System Timers and Paths
extern const uint64_t SYSTEM_POWER_ON;
extern volatile uint64_t USER_POWER_ON;
extern uint64_t SYSTEM_POWER_OFF;
extern const uint64_t SENSOR_TASK_TIMER;

extern const char* JSON_DIR_PATH;
extern const char* CSV_DIR_PATH;

extern unsigned int jsonLength;
// Battery Management
// extern const uint16_t BATTERY_CHARGE;
extern Preferences prefs;

#endif
