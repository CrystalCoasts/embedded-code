#include "io_handler.h"
#include "rtc_handler.h"
#include <HTTPClient.h>
#include <WiFi.h>

// extern bool isConnected;

extern SemaphoreHandle_t sdCardMutex;
extern SemaphoreHandle_t sensorMutex;
const char* DATA_URL = "https://smart-seawall-server-4c5cb6fd8f61.herokuapp.com/api/data";

// Define these in a suitable header file or at the top of your source file
const char* KEY_HUMIDITY = "humidity";
const char* KEY_TEMPERATURE = "temperature";
const char* KEY_TURBIDITY = "turbidity";
const char* KEY_SALINITY = "salinity";
const char* KEY_TDS = "tds";
const char* KEY_PH = "pH";
const char* KEY_OXYGEN_LEVEL = "oxygenLevel";
const char* KEY_MONTH = "Month";
const char* KEY_DAY = "Day";
const char* KEY_YEAR = "Year";
const char* KEY_HOUR = "Hour";
const char* KEY_MINUTE = "Minute";
const char* KEY_SECOND = "Second";


void readSensorData(SensorData &data)
{
    Serial.println("Reading sensor data...");
    data.temperatureValid = temp.readTemperature(CELSIUS, &data.temperature);
    data.turbidityValid = tbdty.readTurbidity(&data.turbidity);
    data.salinityValid = sal.readSalinity(&data.salinity);
    data.tdsValid = sal.readTDS(&data.tds);
    data.pHValid = phGloabl.readpH(&data.pH);
    data.oxygenLevelValid = DO.readDO(&data.oxygenLevel, data.salinity, data.temperature);
    data.humidityValid = temp.readHumidity(&data.humidity);



    // Round readings
    data.temperature = round(data.temperature * 1000.0) / 1000.0;
    data.turbidity = round(data.turbidity * 1000.0) / 1000.0;
    data.salinity = round(data.salinity * 1000.0) / 1000.0;
    data.pH = round(data.pH * 1000.0) / 1000.0;
    data.oxygenLevel = round(data.oxygenLevel * 1000.0) / 1000.0;
    data.tds = round(data.tds * 1000.0) / 1000.0;
    data.humidity = round(data.humidity * 1000.0) / 1000.0;

    Serial.println("Sensor readings complete.");
}


void uploadData(String jsonData) {
    if (WiFi.status() == WL_CONNECTED) {
        // Serial.println("Connected to WiFi. Sending data...");
        Serial.println(jsonData);

        HTTPClient http;
        http.begin(DATA_URL);
        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.POST(jsonData);
        if (httpResponseCode > 0) {
            // Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
        } else {
            Serial.print("[WIFI] Error code: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    }
    //  else {
    //     Serial.println("WiFi is not connected. Skipping data upload.");
    // }
}




void printDataOnCLI(const SensorData& data){
    String toPrint="";
    
    // Header
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint += "|Sensor Readings:\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint += "|Temperature: "+String(data.temperature,3)+"\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint += "|Salinity:"+String(data.salinity,3)+"\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint+= "|Turbidity: "+String(data.turbidity,3)+"\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint+= "|TDS: "+String(data.tds,3)+"\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint+= "|Oxigen Level: "+String(data.oxygenLevel,3)+"\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint+= "|PH: "+String(data.pH,3)+"\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint+= "|Humidity: "+String(data.humidity,3)+"\n";
    toPrint += "+-----------------------+-----------------------+\n";

    Serial.println(toPrint);
}

void validateSensorReadings(SensorData& data) {
    data.humidityValid = !isnan(data.humidity) && data.humidity >= 0 && data.humidity <= 100;
    data.temperatureValid = !isnan(data.temperature) && data.temperature >= -40 && data.temperature <= 85;
    data.turbidityValid = !isnan(data.turbidity) && data.turbidity >= 0;
    data.salinityValid = !isnan(data.salinity) && data.salinity >= 0;
    data.tdsValid = !isnan(data.tds) && data.tds >= 0;
    data.pHValid = !isnan(data.pH) && data.pH >= 0;
    data.oxygenLevelValid = !isnan(data.oxygenLevel) && data.oxygenLevel >= 0;
}



String prepareJsonPayload(const SensorData& data) {
    const tm& timeinfo = get_current_time();
    StaticJsonDocument<256> doc;
    doc[KEY_HUMIDITY] = String(data.humidity, 3);
    doc[KEY_TEMPERATURE] = String(data.temperature, 3);
    doc[KEY_TURBIDITY] = String(data.turbidity, 3);
    doc[KEY_SALINITY] = String(data.salinity, 3);
    doc[KEY_TDS] = String(data.tds, 3);
    doc[KEY_PH] = String(data.pH, 3);
    doc[KEY_OXYGEN_LEVEL] = String(data.oxygenLevel, 3);
    doc[KEY_MONTH] = String(timeinfo.tm_mon+1);
    doc[KEY_DAY] = String(timeinfo.tm_mday);
    doc[KEY_YEAR] = String(timeinfo.tm_year);
    doc[KEY_HOUR] = String(timeinfo.tm_hour);
    doc[KEY_MINUTE] = String(timeinfo.tm_min);
    doc[KEY_SECOND] = String(timeinfo.tm_sec);
    String jsonPayload;
    serializeJson(doc, jsonPayload);
    return jsonPayload;
}

String prepareCSVPayload(const SensorData& data)    {
    const tm& timeinfo = get_current_time();
    return String(data.humidity, 3) + ", " + String(data.temperature, 3) +
        ", " + String(data.turbidity, 3) + ", " + String(data.salinity, 3) + 
        ", " + String(data.tds, 3) + ", " + String(data.pH, 3) + ", " +
        String(data.oxygenLevel, 3) + ", " + (timeinfo.tm_mon+1) + ", " +
        timeinfo.tm_mday + ", " + (timeinfo.tm_year) + ", " + timeinfo.tm_hour +
        ":" + timeinfo.tm_min + ":" + timeinfo.tm_sec;   
}

bool saveCSVData(SdFat32 &SD,const String& data) {
    if (xSemaphoreTake(sdCardMutex, pdMS_TO_TICKS(5000))) {
        struct tm timeinfo = get_current_time(); // Use value, not reference
        Serial.println("Saving data to CSV file...");
        File32 file;

        String filename;
        if(!getLocalTime(&timeinfo)) {
            filename = "/csvFiles/unknown-time.csv"; 
        } else {
            filename = String("/csvFiles/") + (timeinfo.tm_mon+1) + '-' + timeinfo.tm_mday + '-' + (timeinfo.tm_year) + String("-data.csv"); 
        }
        
        file = SD.open(filename, O_WRITE | O_APPEND | O_CREAT);
        if (!file) {
            Serial.println("Failed to open file for writing");
            xSemaphoreGive(sdCardMutex);
            return false;
        }

        if (file.println(data)) {
            Serial.println("Data saved successfully.");
        } else {
            Serial.println("Failed to save data.");
        }
        file.close();
        xSemaphoreGive(sdCardMutex);
        return true;
    } else {
        Serial.println("Failed to obtain SD Card mutex for writing.");
        return false;
    }
}


String readDataFromSD(SdFat32 &SD, const char* fileName) {
    if (xSemaphoreTake(sdCardMutex, pdMS_TO_TICKS(5000))) {
        File32 file = SD.open(fileName, O_READ);
        if (!file) {
            Serial.println("Failed to open file for reading");
            xSemaphoreGive(sdCardMutex);
            return String();
        }

        String data = file.readStringUntil('\n');
        file.close();
        xSemaphoreGive(sdCardMutex);
        return data;
    } else {
        Serial.println("Failed to obtain SD Card mutex for reading.");
        return String();
    }
}



bool saveDataToJSONFile(SdFat32 &SD, const String &data) {
    if (xSemaphoreTake(sdCardMutex, pdMS_TO_TICKS(5000))) { // Attempt to take the mutex with a timeout
        struct tm timeinfo = get_current_time();
        Serial.println("Saving data to JSON file...");
        Serial.println(data);
        File32 file;
        String filename;
        if (!getLocalTime(&timeinfo)) {
            filename = "/jsonFiles/unknown-time.json";
        } else {
            filename = String("/jsonFiles/") + (timeinfo.tm_mon + 1) + '-' + timeinfo.tm_mday + '-' + (timeinfo.tm_year) + String("-data.json");
        }
        file = SD.open(filename, O_WRITE | O_CREAT | O_APPEND);
        if (file) {
            if (file.println(data)) {
                Serial.println("Data saved successfully.");
            } else {
                Serial.println("Failed to save data.");
            }
            file.close();
        } else {
            Serial.println("Failed to open JSON file for writing.");
        }
        xSemaphoreGive(sdCardMutex); // Always release the mutex
        return true;
    } else {
        Serial.println("Failed to obtain SD Card mutex for writing JSON.");
        return false;
    }
}

