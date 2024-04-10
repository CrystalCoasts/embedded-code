#include <Arduino.h>

//required
#include <WiFi.h>
#include <HTTPClient.h>
#include "FS.h"
#include "SPIFFS.h"
#include "esp_sleep.h"

//our classes
#include "TempSensor.h"
#include "LcdDisplay.h"
#include "TurbiditySensor.h"
// #include "config.h"

const char* SSID = "Diane";
const char* PASSWD = "12345678";
const char* WEB_APP_URL = "https://script.google.com/macros/s/AKfycbyP09LCH7aRHxhd2tVU9Q3xyEZBOODTN3lYw031-epCypqzabFI7mX5h6Ue1ONw6iNY/exec";


// Forward declarations
void saveDataToCSV(String data);
void uploadData(String data);
void SensorTask(void *pvParameters);
void dataTransmissionTask(void *pvParameters);
String prepareJsonPayload(const String& csvData);
String readDataFromCSV();

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
    lcd.begin();

    // Initialize WiFi
    WiFi.begin(SSID, PASSWD);

    // Create tasks
    xTaskCreate(SensorTask, "Sensor Task", 4096, NULL, 1, NULL);  // Increase from 2048 to 4096
    xTaskCreate(dataTransmissionTask, "Data Transmission Task", 4096, NULL, 1, NULL);
}

void SensorTask(void *pvParameters) {
    while (1) {
        float humidity = temp.readHumidity();
        float waterTemp = temp.readTemperature(CELSIUS);
        float turbidity = tbdty.readTurbidity();

        // Validate sensor readings
        if (isnan(humidity) || humidity < 0) {
            Serial.println("Invalid humidity reading");
            humidity = 0.0;  // Set a default or error value
        }
        if (isnan(waterTemp) || waterTemp < -40 || waterTemp > 85) {  // Assuming -40 to 85°C is the valid range
            Serial.println("Invalid temperature reading");
            waterTemp = 0.0;  // Set a default or error value
        }
        if (isnan(turbidity) || turbidity < 0) {
            Serial.println("Invalid turbidity reading");
            turbidity = 0.0;  // Set a default or error value
        }

        // Process and display data
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print(" Water Temp: ");
        Serial.print(waterTemp);
        Serial.print(" Turbidity: ");
        Serial.println(turbidity);

        // Save data to CSV
        String dataString = String(waterTemp) + "," + String(humidity) + "," + String(turbidity);
        saveDataToCSV(dataString);

        // Sleep or delay before next read
        vTaskDelay(pdMS_TO_TICKS(5000)); // 5 seconds delay
    }
}


String prepareJsonPayload(const String& csvData) {
    int firstComma = csvData.indexOf(',');
    int secondComma = csvData.indexOf(',', firstComma + 1);
    int thirdComma = csvData.indexOf(',', secondComma + 1);

    String temperature = csvData.substring(0, firstComma);
    String humidity = csvData.substring(firstComma + 1, secondComma);
    String turbidity = csvData.substring(secondComma + 1, thirdComma); // Adjust based on actual data positions

    // Construct the JSON payload
    String jsonPayload = "{\"temperature\":" + temperature +
                         ",\"humidity\":" + humidity +
                         ",\"turbidity\":" + turbidity + "}";
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

void uploadData(String data) {
    HTTPClient http;
    http.begin(WEB_APP_URL);
    http.addHeader("Content-Type", "application/json");
    String httpRequestData = prepareJsonPayload(data);
    int httpResponseCode = http.POST(httpRequestData);
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

void loop(){}
