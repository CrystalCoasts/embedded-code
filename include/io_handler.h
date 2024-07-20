#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

#include <iostream>
#include <Arduino.h>
#include <time.h>
#include "SdFat.h"

#include <ArduinoJson.h>

#include "TempSensor.h"
#include "TurbiditySensor.h"
#include "SalinitySensor.h"
#include "pHSensor.h"
// #include <base_surveyor.h>
#include "DOSensor.h"

struct SensorData {
    float humidity;
    bool humidityValid  = true;
    float temperature ;
    bool temperatureValid= true;
    float turbidity;
    bool turbidityValid= true;
    float salinity;
    bool salinityValid= true;
    float tds;
    bool tdsValid= true;
    float pH ;
    bool pHValid= true;
    float oxygenLevel;
    bool oxygenLevelValid= true;
};

void readSensorData(SensorData& data);
void validateSensorReadings(SensorData& data); 
void printDataOnCLI(const SensorData& data);
bool uploadData(String jsonData);

/** file interactions **/

//json
String prepareJsonPayload(const SensorData& data);
bool saveJsonData(SdFat32 &SD, const String &data);
//csv
String prepareCSVPayload(const SensorData& data);
bool saveCSVData(SdFat32 &SD, const String& data);
String readDataFromSD(SdFat32 &SD, const char* fileName);

bool is_time_synced();


#endif // DATA_HANDLER_H