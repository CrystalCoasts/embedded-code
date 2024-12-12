#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

#include <iostream>
#include <Arduino.h>
#include <time.h>
#include "SdFat.h"
#include <SD_MMC.h>

#include <ArduinoJson.h>

#include "TempSensor.h"
#include "TurbiditySensor.h"
#include "SalinitySensor.h"
#include "pHSensor.h"
// #include <base_surveyor.h>
#include "DOSensor.h"

#define IO_RXD2 47
#define IO_TXD2 48

#define PIN_SD_CMD 11
#define PIN_SD_CLK 12
#define PIN_SD_D0 13



struct SensorData {
    float humidity;
    bool humidityValid  = true;
    float temperature;
    bool temperatureValid= true;
    float turbidity;
    bool turbidityValid= true;
    float salinity;
    bool salinityValid= true;
    float tds;
    bool tdsValid= true;
    float ec;
    bool ecValid = true;
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
bool saveJsonData(fs::FS &fs, const String &data);
//csv
String prepareCSVPayload(const SensorData& data);
bool saveCSVData(fs::FS &fs, const String& data);
String readDataFromSD(fs::FS &fs, const char* fileName);

bool is_time_synced();


#endif // DATA_HANDLER_H