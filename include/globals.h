// globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>

#include "preferences.h"

extern const uint64_t SYSTEM_POWER_ON;
extern volatile uint64_t USER_POWER_ON;
extern uint64_t SYSTEM_POWER_OFF;
extern const uint64_t SENSOR_TASK_TIMER;

extern const char* JSON_DIR_PATH;
extern const char* CSV_DIR_PATH;


// extern Preferences preferences;

#endif
