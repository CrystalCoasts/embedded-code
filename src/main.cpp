#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "FS.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>

// Sensor headers
#include "TempSensor.h"
#include "TurbiditySensor.h"
#include "SalinitySensor.h"

const char* SSID = "seawall";
const char* PASSWD = "12345678";
const char* WEB_APP_URL = "https://script.google.com/macros/s/AKfycbzdREDYLRb1ew8CjwGY_WnrIU0UWW0Sn3Wr4XdT8Jv0VjXuQxJV7GVCKZeYtEb2zrKb/exec";

const int STATUS_LED_PIN = 2; // Commonly the onboard LED pin on ESP32
const int BUTTON_PIN = 0;

// Struct for sensor data
struct SensorData {
    float humidity;
    bool humidityValid;
    float temperature;
    bool temperatureValid;
    float turbidity;
    bool turbidityValid;
    float salinity;
    bool salinityValid;
    float tds;
    bool tdsValid;
    float pH;
    bool pHValid;
    float oxygenLevel;
    bool oxygenLevelValid;
};

// Forward declarations
void saveDataToJSONFile(String data);
void uploadData(String data);
String prepareJsonPayload(const SensorData& data);
String readDataFromJSONFile();
void validateSensorReadings(SensorData& data);
void printDataOnCLI(const SensorData& data);

// Status functions
void blinkLED(int delayTime);
void setLEDSolid(bool on);

void setup() {
    Serial.begin(115200);
    Wire.begin();

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    } else {
        Serial.println("SPIFFS mounted successfully.");
    }

    pinMode(STATUS_LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);

    // Initialize sensors
    temp.begin();
    tbdty.begin();
    tbdty.calibrate();
    sal.begin();
    sal.EnableDisableSingleReading(SAL, 1);

    // Initialize WiFi
    WiFi.begin(SSID, PASSWD);
}

void loop() {
    // Sensor data
    SensorData data = {0};
    temp.readHumidity(&data.humidity);
    temp.readTemperature(FAHRENHEIT, &data.temperature);
    // Serial.print(data.temperature);
    tbdty.readTurbidity(&data.turbidity);
    sal.readSalinity(&data.salinity);

    // Round readings
    data.humidity = round(data.humidity * 1000.0) / 1000.0;
    data.temperature = round(data.temperature * 1000.0) / 1000.0;
    data.turbidity = round(data.turbidity * 1000.0) / 1000.0;
    data.salinity = round(data.salinity * 1000.0) / 1000.0;

    // Default values for other sensors
    data.tds = 111.0;
    data.tdsValid = true;
    data.pH = 7.0;
    data.pHValid = true;
    data.oxygenLevel = 36.0;
    data.oxygenLevelValid = true;

    // Validate readings
    validateSensorReadings(data);

    String jsonPayload = prepareJsonPayload(data);
    printDataOnCLI(data);

    if (WiFi.status() == WL_CONNECTED) {
        uploadData(jsonPayload);
    } else {
        saveDataToJSONFile(jsonPayload);
    }

    // Handle button press for turbidity sensor calibration
    if (digitalRead(BUTTON_PIN) == LOW) {
        tbdty.calibrate();
        // Simple debounce
        while (digitalRead(BUTTON_PIN) == LOW) {
            delay(50);
        }
    }

    // Update LED status based on WiFi connection
    if (WiFi.status() == WL_CONNECTED) {
        setLEDSolid(true);
    } else {
        blinkLED(500);
    }

    delay(5000); // 5 seconds delay for next sensor read
}

void printDataOnCLI(const SensorData& data){
    String toPrint="";
    
    // Header
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint += "|\tSensor\t\t|\tReading\t\t|\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint += "|\tTemperature\t|\t"+String(data.temperature,3)+"\t\t|\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint += "|\tSalinity\t|\t"+String(data.salinity,3)+"\t\t|\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint+= "|\tTurbidity\t|\t"+String(data.turbidity,3)+"\t|\n";
    toPrint += "+-----------------------+-----------------------+\n";

    Serial.println(toPrint);
}

String prepareJsonPayload(const SensorData& data) {
    StaticJsonDocument<256> doc;
    if (data.humidityValid) doc["humidity"] = String(data.humidity, 3);
    if (data.temperatureValid) doc["temperature"] = String(data.temperature, 3);
    if (data.turbidityValid) doc["turbidity"] = String(data.turbidity, 3);
    if (data.salinityValid) doc["salinity"] = String(data.salinity, 3);
    if (data.tdsValid) doc["TDS"] = String(data.tds, 3);
    if (data.pHValid) doc["pH"] = String(data.pH, 3);
    if (data.oxygenLevelValid) doc["oxygenLevel"] = String(data.oxygenLevel, 3);

    String jsonPayload;
    serializeJson(doc, jsonPayload);
    return jsonPayload;
}

void saveDataToJSONFile(String data) {
    Serial.println("Saving data to JSON file...");

    // Serial.println(data);
    File file = SPIFFS.open("/data.json", FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.println(data)) {
        Serial.println("Data saved successfully.");
    } else {
        Serial.println("Failed to save data.");
    }
    file.close();
}

String readDataFromJSONFile() {
    File file = SPIFFS.open("/data.json");
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
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Sending to Google.");
        // Serial.print(jsonData);
        HTTPClient http;
        http.begin(WEB_APP_URL);
        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.POST(jsonData);
        http.end();
    } else {
        Serial.println("WiFi is not connected. Skipping data upload.");
    }
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

void blinkLED(int delayTime) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(delayTime);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(delayTime);
}

void setLEDSolid(bool on) {
    digitalWrite(STATUS_LED_PIN, on ? HIGH : LOW);
}
