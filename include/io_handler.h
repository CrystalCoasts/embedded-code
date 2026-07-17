#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H


#include <iostream>
#include <Arduino.h>
#include <time.h>
#include "SD.h"
#include "ArduinoJson.h"

#include "TempSensor.h"
#include "TurbiditySensor.h"
#include "SalinitySensor.h"
#include "pHSensor.h"
// #include <base_surveyor.h>
#include "DOSensor.h"
#include "esp_heap_caps.h"
#include "AtlasTemp.h"
#include "AtlasPH.h"

#define IO_RXD2 47
#define IO_TXD2 48

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
    unsigned int jsonLength;
};

/** Sensor reading and uploading **/
void readSensorData(SensorData& data);
void validateSensorReadings(SensorData& data); 
void printDataOnCLI(const SensorData& data);
bool uploadData(String jsonData);

/** Cellular Connectivity **/
std::string sendData(String command);
void modemRestart();
void modemPowerOff();
void modemPowerOn();
void sendPostRequest(String jsonPayload, const char* server, const char* resource);
void sendGetRequest();

/** file interactions **/

//json
String prepareJsonPayload(const SensorData& data);
String prepareJsonPayload(const SensorData& data, const struct tm& timeinfo);
bool saveJsonData(fs::FS &fs, const String &data);
bool saveJsonData(fs::FS &fs, const String &data, const struct tm& timeinfo);

//csv
String prepareCSVPayload(const SensorData& data);
String prepareCSVPayload(const SensorData& data, const struct tm& timeinfo);
bool saveCSVData(fs::FS &fs, const String& data);
bool saveCSVData(fs::FS &fs, const String& data, const struct tm& timeinfo);
String readDataFromSD(fs::FS &fs, const char* fileName);

bool is_time_synced();


#endif // DATA_HANDLER_H