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
    float humidity = 0;
    bool humidityValid  = true;
    float temperature = 0;
    bool temperatureValid= true;
    float turbidity = 0;
    bool turbidityValid= true;
    float salinity = 0;
    bool salinityValid= true;
    float tds = 111;
    bool tdsValid= true;
    float pH = 0;
    bool pHValid= true;
    float oxygenLevel=0;
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