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
    float temperature;
    bool temperatureValid= true;
    float turbidity;
    bool turbidityValid= true;
    float salinity;
    bool salinityValid= true;
    float tds ;
    bool tdsValid= true;
    float pH ;
    bool pHValid= true;
    float oxygenLevel;
    bool oxygenLevelValid= true;
};

void readSensorData(SensorData& data);
void validateSensorReadings(SensorData& data); 
void printDataOnCLI(const SensorData& data);
void uploadData(String jsonData);

/** file interactions **/

//json
String prepareJsonPayload(const SensorData& data);
void saveDataToJSONFile(SdFat32 &SD,String data); 
String readDataFromJSONFile();

//csv
String prepareCSVPayload(const SensorData& data);
void saveCSVData(SdFat32 &SD, String data);

//for testing
bool is_time_synced();


#endif // DATA_HANDLER_H