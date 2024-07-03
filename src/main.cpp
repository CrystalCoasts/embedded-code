#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include "SPI.h"
#include "FS.h"
#include "SdFat.h"

SdFat32 SD;

// Sensor headers
#include "TempSensor.h"
#include "TurbiditySensor.h"
#include "SalinitySensor.h"
#include <ph_surveyor.h>
#include <base_surveyor.h>
#include "DOSensor.h"

const char* SSID = "seawall";
const char* PASSWD = "12345678";
const char* WEB_APP_URL = "https://script.google.com/macros/s/AKfycbzdREDYLRb1ew8CjwGY_WnrIU0UWW0Sn3Wr4XdT8Jv0VjXuQxJV7GVCKZeYtEb2zrKb/exec";
const char *serverName = "https://smart-seawall-server-4c5cb6fd8f61.herokuapp.com/";
//const char *serverName = "MONGODB_URI=mongodb+srv://lisettehawkins09:cxO0hBBXellzkuAX@cluster0-sensordatassam.sk9l59s.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0-sensorDatasSample";

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

Surveyor_pH pH = Surveyor_pH(35);

// Forward declarations
void saveDataToJSONFile(String data);
void uploadData(String data);
String prepareJsonPayload(const SensorData& data);
String readDataFromJSONFile();
void validateSensorReadings(SensorData& data);
void printDataOnCLI(const SensorData& data);
void sdBegin();

// Status functions
void blinkLED(int delayTime);
void setLEDSolid(bool on);
bool cardMount = false;

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
    // tbdty.calibrate();
    sal.begin();
    sal.EnableDisableSingleReading(SAL, 1);
    //sal.EnableDisableSingleReading(TDS,1);
    DO.begin();

    SPI.begin(18, 19, 23, 5);
    SPI.setDataMode(SPI_MODE0);
    if(!SD.begin(SdSpiConfig(5, SHARED_SPI, SD_SCK_MHZ(16)))){
        Serial.println("Card Mount Failed");
        
    }else   {
        Serial.println("Card mount sucessful!");
        cardMount = true;
    } 

    // Initialize WiFi
    WiFi.begin(SSID, PASSWD);    

}

void loop() {
    if(cardMount != true)   {
        if(!SD.begin(SdSpiConfig(5, SHARED_SPI, SD_SCK_MHZ(16)))){
            Serial.println("Card Mount Failed");
        }else   {
            Serial.println("Card mount sucessful!");
            cardMount = true;
        }   

    }
    // Sensor data
    SensorData data = {0};
    data.humidityValid = temp.readHumidity(&data.humidity);
    data.temperatureValid  = temp.readTemperature(CELSIUS, &data.temperature);
    // Serial.print(data.temperature);
    data.turbidityValid = tbdty.readTurbidity(&data.turbidity);
    data.salinityValid = sal.readSalinity(&data.salinity);
    data.pH = pH.read_ph();
    data.oxygenLevelValid = DO.readDO(&data.oxygenLevel, data.salinity, data.temperature);

    // Round readings
    data.humidity = round(data.humidity * 1000.0) / 1000.0;
    data.temperature = round(data.temperature * 1000.0) / 1000.0;
    data.turbidity = round(data.turbidity * 1000.0) / 1000.0;
    data.salinity = round(data.salinity * 1000.0) / 1000.0;

    data.pH = round(data.pH * 1000.0)/1000.0;

    // Default values for other sensors
    data.tds = 111.0;
    //data.tdsValid = true;
    //data.pH = 7.0;
    data.pHValid = true;
    //data.oxygenLevel = 36.0;
    //data.oxygenLevelValid = true;

    // Validate readings
    //validateSensorReadings(data);

    String jsonPayload = prepareJsonPayload(data);
    printDataOnCLI(data);

    if (WiFi.status() == WL_CONNECTED) {
        uploadData(jsonPayload);
    } else {
        saveDataToJSONFile(jsonPayload);
    }

    //saveSensorRecord("12345678");

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
    toPrint += "|Sensor Readings:\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint += "|Temperature: "+String(data.temperature,3)+"\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint += "|Salinity:"+String(data.salinity,3)+"\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint += "|Turbidity: "+String(data.turbidity,3)+"\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint += "pH Value: "+String(data.pH,3) + "\n";
    toPrint += "Humidity Value: "+String(data.humidity,3) + "\n";
    toPrint += "Oxygen Value: "+String(data.oxygenLevel,3) + "\n";

    Serial.println(toPrint);
}

String prepareJsonPayload(const SensorData& data) {
    StaticJsonDocument<256> doc;
    doc["humidity"] = String(data.humidity, 3);
    doc["temperature"] = String(data.temperature, 3);
    doc["turbidity"] = String(data.turbidity, 3);
    doc["salinity"] = String(data.salinity, 3);
    doc["TDS"] = String(data.tds, 3);
    doc["pH"] = String(data.pH, 3);
    doc["oxygenLevel"] = String(data.oxygenLevel, 3);

    String jsonPayload;
    serializeJson(doc, jsonPayload);
    return jsonPayload;
}

void saveDataToJSONFile(String data) {
    // Serial.println("Saving data to JSON file...");
    // Serial.println(data);
    // File32 file = SD.open("/test.txt", O_WRITE | O_CREAT | O_APPEND);
    // if (file.println(data)) {
    //     Serial.println("Data saved successfully.");
    // } else {
    //     Serial.println("Failed to save data.");
    // }
    // file.close();
}

String readDataFromJSONFile() {
    // File file = SPIFFS.open("/data.json");
    // if (!file) {
    //     Serial.println("There was an error opening the file for reading");
    //     return "";
    // }
    // String data;
    // while (file.available()) {
    //     data += file.readStringUntil('\n') + "\n";
    // }
    // file.close();
    // return data;
}

void uploadData(String jsonData) {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Sending to Google.");
        // Serial.print(jsonData);
        HTTPClient http;
        http.begin(WEB_APP_URL);
        int httpResponseCode = http.POST(jsonData);
        http.addHeader("Content-Type", "application/json");
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

