#include <Arduino.h>

//required
#include <WiFi.h>
#include <HTTPClient.h>
#include "FS.h"
#include "SPIFFS.h" //SPI Flash File System -- basically on board memory
#include "esp_sleep.h"
#include <ArduinoJson.h>

//our classes
#include "TempSensor.h"
#include "LcdDisplay.h"
#include "TurbiditySensor.h"
#include "AtlasKit.h"
// #include "config.h"

const char* SSID = "Diane";
const char* PASSWD = "12345678";
const char* WEB_APP_URL = "https://script.google.com/macros/s/AKfycbzdREDYLRb1ew8CjwGY_WnrIU0UWW0Sn3Wr4XdT8Jv0VjXuQxJV7GVCKZeYtEb2zrKb/exec";


// Forward declarations
void saveDataToCSV(String data);
void uploadData(String data);
void SensorTask(void *pvParameters);
void dataTransmissionTask(void *pvParameters);
String prepareJsonPayload(float pH, float oxygenLevel, float salinity, float turbidity, float tds, float temperature);
String readDataFromCSV();
void validateSensorReadings(float& humidity, float& temperature, float& turbidity, float& salinity, float& tds);

void setup() {
    // General setup
    Serial.begin(115200);

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return; // Stop further execution if SPIFFS fails to initialize
    } else {
        Serial.println("SPIFFS mounted successfully.");
    }

    // Initialize sensors
    temp.begin();
    tbdty.begin();
    qSensor.begin();
    lcd.begin();

    // Initialize WiFi
    WiFi.begin(SSID, PASSWD);

    // Create tasks
    xTaskCreate(SensorTask, "Sensor Task", 4096, NULL, 1, NULL);  // Increase from 2048 to 4096
    xTaskCreate(dataTransmissionTask, "Data Transmission Task", 4096, NULL, 1, NULL);
}

void SensorTask(void *pvParameters) {
    while (1) {

        //these should get destroyed when loop ends and recreated... shouldn't affect memory
        float humidity = temp.readHumidity();
        float waterTemp = temp.readTemperature(CELSIUS);
        float turbidity = tbdty.readTurbidity();
        float salinity = qSensor.read(MType::SAL); // Read salinity
        float tds = qSensor.read(MType::TDS); // Read TDS

        float pH = 0.0;
        float oxygenLevel = 0.0;
        
        validateSensorReadings(humidity, waterTemp, turbidity, salinity, tds);
        
        String jsonPayload = prepareJsonPayload(pH, oxygenLevel, salinity, turbidity, tds, waterTemp);
        uploadData(jsonPayload); // Now passing the correct JSON string

        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print(", Water Temp: ");
        Serial.print(waterTemp);
        Serial.print(", Turbidity: ");
        Serial.print(turbidity);
        Serial.print(", Salinity: ");
        Serial.print(salinity);
        Serial.print(", TDS: ");
        Serial.println(tds);


        // Sleep or delay before next read
        vTaskDelay(pdMS_TO_TICKS(5000)); // 5 seconds delay
    }
}


String prepareJsonPayload(float pH, float oxygenLevel, float salinity, float turbidity, float tds, float temperature) {
    StaticJsonDocument<256> doc;
    doc["pH"] = isnan(pH) ? JsonVariant() : pH;
    doc["oxygenLevel"] = isnan(oxygenLevel) ? JsonVariant() : oxygenLevel;
    doc["salinity"] = isnan(salinity) ? JsonVariant() : salinity;
    doc["turbidity"] = isnan(turbidity) ? JsonVariant() : turbidity;
    doc["TDS"] = isnan(tds) ? JsonVariant() : tds;
    doc["temperature"] = isnan(temperature) ? JsonVariant() : temperature;

    String jsonPayload;
    serializeJson(doc, jsonPayload);
    return jsonPayload;
}


void saveDataToCSV(String data) {
    // Serial.println("Attempting to save data...");
    File file = SPIFFS.open("/data.csv", FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    file.close();
    vTaskDelay(pdMS_TO_TICKS(100)); 
}


String readDataFromCSV() {
    File file = SPIFFS.open("/data.csv");
    if (!file) {
        Serial.println("There was an error opening the file for reading");
        return "";
    }
    String data;
    while (file.available()) {
        data += file.readStringUntil('\n') + "\n";
    }
    file.close();
    return data;
}


void uploadData(String jsonData) {
    HTTPClient http;
    http.begin(WEB_APP_URL);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonData);
    if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        SPIFFS.remove("/data.csv"); // Delete the file if upload successful
    } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    http.end();
}


void dataTransmissionTask(void *pvParameters) {
    while (1) {
        if (WiFi.status() == WL_CONNECTED) {
            if (SPIFFS.exists("/data.csv")) {
                String data = readDataFromCSV();
                uploadData(data);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10000)); // Delay to prevent excessive retries
    }
}

void validateSensorReadings(float& humidity, float& temperature, float& turbidity, float& salinity, float& tds) {
    // Check for NaN values or any other sensor-specific invalid data
    if (isnan(humidity) || humidity < 0 || humidity > 100) {  // Humidity should be between 0% and 100%
        Serial.println("Invalid humidity reading");
        humidity = 0.0;  
    }
    if (isnan(temperature) || temperature < -40 || temperature > 85) {  // Check reasonable bounds for temperature
        Serial.println("Invalid temperature reading");
        temperature = 0.0;  
    }
    if (isnan(turbidity) || turbidity < 0) {  // Turbidity should not be negative
        Serial.println("Invalid turbidity reading");
        turbidity = 0.0;  
    }
    if (isnan(salinity) || salinity < 0) {  // Salinity should not be negative
        Serial.println("Invalid salinity reading");
        salinity = 0.0;  
    }
    if (isnan(tds) || tds < 0) {  // TDS (Total Dissolved Solids) should not be negative
        Serial.println("Invalid TDS reading");
        tds = 0.0;  
    }

    //maybe make it bool and use it to decide if data will be sent to .csv or not
}


void loop(){}
