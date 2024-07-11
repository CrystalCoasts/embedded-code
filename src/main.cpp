#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include "SPI.h"
#include "FS.h"
#include "SdFat.h"
#include "NTPClient.h"
#include "WiFiUdp.h"
#include "esp_sntp.h"

// Sensor headers
#include "TempSensor.h"
#include "TurbiditySensor.h"
#include "SalinitySensor.h"
#include "pHSensor.h"
#include <base_surveyor.h>
#include "DOSensor.h"

#include "io_handler.h"
#include "rtc_handler.h"

#define SLEEP_TIME_US 30000000 // 1 minute = 60000000 - multiply by any number for amount of minutes

SdFat32 SD;

const char* SSID = "seawall";
const char* PASSWD = "12345678";
const char* WEB_APP_URL = "https://script.google.com/macros/s/AKfycbzdREDYLRb1ew8CjwGY_WnrIU0UWW0Sn3Wr4XdT8Jv0VjXuQxJV7GVCKZeYtEb2zrKb/exec";
const char *serverName = "https://smart-seawall-server-4c5cb6fd8f61.herokuapp.com/api/data";
//const char *serverName = "MONGODB_URI=mongodb+srv://lisettehawkins09:cxO0hBBXellzkuAX@cluster0-sensordatassam.sk9l59s.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0-sensorDatasSample";

const int STATUS_LED_PIN = 2; // Commonly the onboard LED pin on ESP32
const int BUTTON_PIN = 0;

Surveyor_pH pH = Surveyor_pH(35);

// void uploadData(String data);

void sdBegin();

// Status functions
void blinkLED(int delayTime);
void setLEDSolid(bool on);
bool cardMount = false;
bool isConnected = false;

//Sleep status for night mode, regular mode, and within 5-10 minutes after supposed connection
enum sleepStatus    {
    regular = 1,    
    withinInterval,
    nightMode,
    connectedToDevice
};

TaskHandle_t taskHandleDataUpload;
SemaphoreHandle_t sdHandler;

void setup() {
    setCpuFrequencyMhz(80);
    Serial.begin(115200);
    Wire.begin();
    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    } else {
        Serial.println("SPIFFS mounted successfully.");
    }

    // pinMode(STATUS_LED_PIN, OUTPUT);
    // pinMode(BUTTON_PIN, INPUT);

    // Initialize sensors
    temp.begin();
    tbdty.begin();
    //tbdty.calibrate();
    sal.begin();
    // sal.EnableDisableSingleReading(SAL, 1);
    //sal.EnableDisableSingleReading(TDS,1);
    DO.begin();
    phGloabl.begin();

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

    rtc_begin();

    //freeRTOS testing jajaja
    sdHandler = xSemaphoreCreateBinary();       //create a binarey semaphore

    xTaskCreate(
        handleDataUpload,       // function name of the task
        "Upload Data 2",   // name of the task (for debugging)
        2048,              // stack size (bytes)
        NULL,              //Parameter to pass
        1,                 //Task priority
        &taskHandleDataUpload   //Task handle
    );
    vTaskSuspend(taskHandleDataUpload);
    xSemaphoreGive(sdHandler);
}

void loop() {
    isConnected = WiFi.status() == WL_CONNECTED;
    if(cardMount != true)   {
        if(!SD.begin(SdSpiConfig(5, SHARED_SPI, SD_SCK_MHZ(16)))){
            Serial.println("Card Mount Failed");
        }else   {
            Serial.println("Card mount sucessful!");
            cardMount = true;
        }   
    }
   
    // Sensor data
    SensorData data;   
    readSensorData(data);

    // Validate readings
    //validateSensorReadings(data);
    if(WiFi.status)    
        vTaskResume(handleDataUpload);

    String jsonPayload = prepareJsonPayload(data);
    String csvPayLoad = prepareCSVPayload(data);
    printDataOnCLI(data);


    // jsonPayload= prepareJsonPayload(data);
    // csvPayLoad = prepareCSVPayload(data);
    if(xSemaphoreTake(sdHandler, portMAX_DELAY))    {
        saveDataToJSONFile(SD, jsonPayload);
        saveCSVData(SD, csvPayLoad);
    }else
        Serial.println("SD resource is taken, skipping data upload");
    

    uploadData(jsonPayload);
    printLocalTime();


}

void handleDataUpload(void *parameter)   {
    while(1)    {
        xSemaphoreTake(sdHandler, portMAX_DELAY);
        
        File32 file = SD.open("/jsonData");
        File32 entry = file.openNextFile();
        while(entry!=EOF)   {
            uploadData2(readDataFromSD(entry.name()));
            if(WiFi.status == 0)    {
                entry.close();
                file.close();
                vTaskSuspend(handleDataUpload);
                break;
            }
        }

        xSemaphoreGive(sdHandler);
        delay(100);
        
    }
}

String readDataFromSD(const char* fileName) {
    File32 file = SD.open("/jsonData");
    if (!file) {
        Serial.println("Failed to open file for reading");
        return String();
    }
    String data = file.readStringUntil('\n');
    file.close();
    return data;
}

void uploadData2(const String& data) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverName);
        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.POST(data);
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("HTTP Response code: " + httpResponseCode);
            Serial.println("Response: " + response);
        } else {
            Serial.println("Error on sending POST: " + httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("WiFi is not connected. Skipping data upload.");
    }
}



// void uploadData(String jsonData) {
//     if (WiFi.status() == WL_CONNECTED) {
//         Serial.println("Sending to Google.");
//         // Serial.print(jsonData);
//         HTTPClient http;
//         http.begin(serverName);
//         int httpResponseCode = http.POST(jsonData);
//         http.addHeader("Content-Type", "application/json");
//         http.end();
//     } else {
//         Serial.println("WiFi is not connected. Skipping data upload.");
//     }
// }


void blinkLED(int delayTime) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(delayTime);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(delayTime);
}

void setLEDSolid(bool on) {
    digitalWrite(STATUS_LED_PIN, on ? HIGH : LOW);
}

