#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>
#include "Preferences.h"

// System Timers and Paths
extern uint64_t SYSTEM_POWER_ON;
extern volatile uint64_t USER_POWER_ON;
extern uint64_t SYSTEM_POWER_OFF;
extern const uint64_t SENSOR_TASK_TIMER;

extern const char* JSON_DIR_PATH;
extern const char* CSV_DIR_PATH;

extern unsigned int jsonLength;

// Battery Management
// extern const uint16_t BATTERY_CHARGE;
extern Preferences prefs;

// global values for sensor config
// Fallbacks for the IDE Linter (Clang/IntelliSense)
// The actual build will use the values from the .env file.
// constant for the id of the sensor array
#ifndef SENSOR_ARRAY_ID
#define SENSOR_ARRAY_ID "LINTER_ARRAY_ID"
#endif
// constant for the ids and types of all sensors
// humidity sensor
#ifndef HUM_SENSOR_ID
#define HUM_SENSOR_ID "LINTER_HUM_ID"
#endif
#ifndef HUM_SENSOR_TYPE
#define HUM_SENSOR_TYPE "LINTER_HUM_TYPE"
#endif
// temperature sensor
#ifndef TEMP_SENSOR_ID
#define TEMP_SENSOR_ID "LINTER_TEMP_ID"
#endif
#ifndef TEMP_SENSOR_TYPE
#define TEMP_SENSOR_TYPE "LINTER_TEMP_TYPE"
#endif
// turbidity sensor
#ifndef TURB_SENSOR_ID
#define TURB_SENSOR_ID "LINTER_TURB_ID"
#endif
#ifndef TURB_SENSOR_TYPE
#define TURB_SENSOR_TYPE "LINTER_TURB_TYPE"
#endif
// salinity sensor
#ifndef SAL_SENSOR_ID
#define SAL_SENSOR_ID "LINTER_SAL_ID"
#endif
#ifndef SAL_SENSOR_TYPE
#define SAL_SENSOR_TYPE "LINTER_SAL_TYPE"
#endif
// electrical coductivity sensor
#ifndef EC_SENSOR_ID
#define EC_SENSOR_ID "LINTER_EC_ID"
#endif
#ifndef EC_SENSOR_TYPE
#define EC_SENSOR_TYPE "LINTER_EC_TYPE"
#endif
// total dissolved solids sensor
#ifndef TDS_SENSOR_ID
#define TDS_SENSOR_ID "LINTER_TDS_ID"
#endif
#ifndef TDS_SENSOR_TYPE
#define TDS_SENSOR_TYPE "LINTER_TDS_TYPE"
#endif
// ph sensor
#ifndef PH_SENSOR_ID
#define PH_SENSOR_ID "LINTER_PH_ID"
#endif
#ifndef PH_SENSOR_TYPE
#define PH_SENSOR_TYPE "LINTER_PH_TYPE"
#endif
// dissolved oxygen sensor
#ifndef DO_SENSOR_ID
#define DO_SENSOR_ID "LINTER_DO_ID"
#endif
#ifndef DO_SENSOR_TYPE
#define DO_SENSOR_TYPE "LINTER_DO_TYPE"
#endif

#endif
