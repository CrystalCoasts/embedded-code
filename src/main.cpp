#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <Preferences.h>
#include <esp_sleep.h>



// Sensor headers
#include "TempSensor.h"
#include "TurbiditySensor.h"
#include "SalinitySensor.h"
#include "pHSensor.h"

//helpers
#include "io_handler.h" //this includes SdFat32
#include "rtc_handler.h" // includes ntp related headers
#include "websockets.h"

// error tags
#define SD_TAG "[SD_CARD]"
#define WIFI_TAG "[WIFI]"
#define SENSOR_TAG "[SENSORS]"
#define MAIN_TAG "[MAIN]"

// FOR TESTING
#define QUARTER_MINUTE_MS (MINUTE_MS / 4)
#define HALF_MINUTE_US (MINUTE_US / 2)
#define QUARTER_MINUTE_US (MINUTE_US / 4)
#define HALF_MINUTE_MS (MINUTE_MS / 2)
#define LED_PIN 2  
#define BATTERY_PIN 27

//instance declarations
const char* SSID = "seawall";
const char* PASSWD = "12345678";
// const char* SSID = "RDF_Rapture";
// const char* PASSWD = "WiFi4RDF*!";
// IPAddress staticIP(192, 168, 254, 100); // Change this to your desired static IP
// IPAddress gateway(192, 168, 50, 1);    // Router's IP address
// IPAddress subnet(255, 255, 255, 0);   // Subnet mask
SdFat32 SD;
// Preferences preferences;

//status variables
bool cardMount = false;
bool isConnected = false;

// global variables
const char* JSON_DIR_PATH = "/jsonFiles";
const char* CSV_DIR_PATH = "/csvFiles";

//timers
// volatile uint64_t powerOnTimer = (3600 * 1000) * 2;  // 2 hours
const uint64_t SYSTEM_POWER_ON = 2 * MINUTE_US;
volatile uint64_t USER_POWER_ON = 5 * MINUTE_US;

uint64_t SYSTEM_POWER_OFF = 1* MINUTE_MS;  
const uint64_t SENSOR_TASK_TIMER = HALF_MINUTE_MS; // 30 seconds, for tasks

//tasks semaphores
SemaphoreHandle_t sdCardMutex;
SemaphoreHandle_t sensorMutex;

//handlers and running status
TimerHandle_t shutdownTimerHandle;
TaskHandle_t TaskSensorHandle = NULL;
TaskHandle_t TaskUploadDataHandle = NULL;
bool sensorTaskRunning = false;
bool webSocketTaskRunning = false;
bool uploadDataTaskRunning= false;

//task functions and callbacks
void sensorTask(void *pvParameters);
void uploadTask(void *pvParameters);
void wifiCheckTask(void *pvParameters);
void shutdownTimerCallback(TimerHandle_t xTimer) ; //callback
void reconnectWiFi();
void onWifiConnect(WiFiEvent_t event, WiFiEventInfo_t info);
void onWifiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info);

//helper functions
void startUploadTask();
void stopUploadTask();
void stopSensorTask();
void powerOffSequence();

void setup() {
    setCpuFrequencyMhz(80);
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT); 
    pinMode(BATTERY_PIN, INPUT);
    
    Wire.begin(); // initialize early to ensure sensors can use it

    //setup spi 
    SPI.begin(18, 19, 23, 5);
    SPI.setDataMode(SPI_MODE0);

    // Initialize sensors before wifi
    temp.begin();
    tbdty.begin();
    phGloabl.begin();
    DO.begin();
    sal.begin(); //also tds

    // Create mutexes
    sdCardMutex = xSemaphoreCreateMutex();
    sensorMutex = xSemaphoreCreateMutex();
    
    // Initialize SD card
    if(!SD.begin(SdSpiConfig(5, SHARED_SPI, SD_SCK_MHZ(16)))){
        String msg = SD_TAG + String (" Card Mount Failed");
        Serial.println(msg);
    }
    else  {
        String msg = SD_TAG + String (" Card mount sucessful!");
        Serial.println(msg);
        cardMount = true;
    }  
    // Initialize WiFi we need to continue even if wifi fails
    WiFi.begin(SSID, PASSWD);
    if (WiFi.status() == WL_CONNECTED){
        String msg = SD_TAG + String (" WiFi connected");
        Serial.println(msg);
        isConnected = true;
    }
  
    //other system initializations
    rtc_begin();
    ws.init();
    // update user wakeup time USER_WAKEUP = (user_wakeup - SYSTEM_WAKEUP > 0 ) ? (user_wakeup - SYSTEM_WAKEUP ) : max uint64_t

    //create tasks and setup powerOff timer
    xTaskCreate(sensorTask, "Sensor Task", 8192, NULL, 1, &TaskSensorHandle);
    shutdownTimerHandle = xTimerCreate("ShutdownTimer", pdMS_TO_TICKS(SYSTEM_POWER_OFF), pdFALSE, (void*) 0, shutdownTimerCallback);
    xTimerStart(shutdownTimerHandle, 0);
    digitalWrite(LED_PIN, HIGH); 
    startUploadTask();

    printLocalTime();

    String msg = MAIN_TAG + String (" Setup done");
    Serial.println(msg);
}



void loop() {
    // uint8_t batteryLevel = digitalRead(BATTERY_PIN);
    // Serial.println("Battery Level: " + String(batteryLevel));
    // delay(1000);
}

/* TASKS */
void sensorTask(void *pvParameters) {
    sensorTaskRunning = true;
    while (sensorTaskRunning) {
        Serial.println("[TASKS] Sensor task running");
        SensorData data;
        readSensorData(data);
        printDataOnCLI(data);
        
        if (!saveCSVData(SD, prepareCSVPayload(data))) {
            Serial.println("[TASKS] Failed to save CSV data.");
        }
        if (!saveJsonData(SD, prepareJsonPayload(data))) {
            Serial.println("[TASKS] Failed to save JSON data.");
        }

        Serial.println("[TASKS] Sensor task sleeping");
        vTaskDelay(pdMS_TO_TICKS(SENSOR_TASK_TIMER));  // Delay the task for SENSOR_TASK_TIMER seconds
    }
    // Clean up or prepare to delete task
    sensorTaskRunning = false;
    vTaskDelete(NULL); // Optionally delete the task explicitly
}

void uploadTask(void *pvParameters) {
    for (;;) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi not connected. Skipping upload.");
            vTaskDelay(pdMS_TO_TICKS(5000)); // Delay before next execution cycle
            continue;
        }

        SdFile root, file;
        if (!root.open(JSON_DIR_PATH, O_READ)) {
            Serial.println("Failed to open directory");
            vTaskDelay(pdMS_TO_TICKS(10000)); // Wait for 10 seconds before retrying
            continue;
        }

        char fileName[100];
        while (file.openNext(&root, O_READ)) {
            if (file.isDir()) {
                file.close();
                continue;
            }

            file.getName(fileName, sizeof(fileName));
            if (String(fileName).startsWith(".") || !String(fileName).endsWith(".json")) {
                file.close();
                file.remove();
                continue;
            }

            bool allLinesUploaded = true;
            String jsonLine;
            char ch;
            while (file.available()) {
                ch = file.read();
                if (ch == '\n' || !file.available()) {
                    if (!jsonLine.isEmpty()) {
                        jsonLine.trim();
                        if (!uploadData(jsonLine)) {
                            Serial.println("Failed to upload: " + jsonLine);
                            allLinesUploaded = false;
                            break;
                        }
                        jsonLine = ""; // Reset the line buffer
                    }
                } else {
                    jsonLine += ch;
                }
            }

            file.close();
            if (allLinesUploaded) {
                SD.remove(String(JSON_DIR_PATH) + "/" + String(fileName)); // Ensure the path is correct
                Serial.println(String(fileName) + " uploaded and deleted successfully.");
            } else {
                Serial.println("Not all lines in the file were uploaded successfully.");
            }
        }
        root.close();
        vTaskDelay(pdMS_TO_TICKS(10000)); // Delay before next execution cycle
    }
}



/* SYSTEM CALLBACKS */
// Timer callback function
void shutdownTimerCallback(TimerHandle_t xTimer) {
    Serial.println("Power-off timer expired. Checking active connection.");
    if (WiFi.status() == WL_CONNECTED) {
        // If connected, reset the timer
        Serial.println("Active WiFi connection detected. Resetting power-off timer.");
        xTimerReset(shutdownTimerHandle, 0);  // Reset the timer to delay shutdown
    } else {
        // No active connection, proceed to power off
        Serial.println("No active connection. Initiating shutdown sequence.");
        powerOffSequence();
    }
}


/* HELPER FUNCTIONS */
void powerOffSequence() {
    Serial.println("Powering off...");
    // Load settings, stop tasks, and prepare for shutdown
    loadTimerSettings();
    stopSensorTask();
    stopUploadTask();
    ws.stop();

    // Calculate next wakeup time and adjust USER_POWER_ON
    // Serial.println(USER_POWER_ON);
    // Serial.println(SYSTEM_POWER_ON);
    // Serial.println((USER_POWER_ON < SYSTEM_POWER_ON));
    uint64_t power_on = (USER_POWER_ON < SYSTEM_POWER_ON) ? USER_POWER_ON : SYSTEM_POWER_ON;
    USER_POWER_ON = (USER_POWER_ON > SYSTEM_POWER_ON) ? (USER_POWER_ON - SYSTEM_POWER_ON) : UINT64_MAX;
    
    // Save settings and power off
    saveTimerSettings(USER_POWER_ON);
    esp_sleep_enable_timer_wakeup(power_on);
    digitalWrite(LED_PIN, LOW);
    esp_deep_sleep_start();
}

void startUploadTask() {
    if (TaskUploadDataHandle == NULL) {
        xTaskCreate(uploadTask, "UploadData", 8192, NULL, 1, &TaskUploadDataHandle);
        uploadDataTaskRunning = true;
    }
}

void stopUploadTask() {
    if (TaskUploadDataHandle != NULL) {
        vTaskDelete(TaskUploadDataHandle);
        uploadDataTaskRunning = false;
        TaskUploadDataHandle = NULL;
    }
}

void stopSensorTask() {
    if (sensorTaskRunning) {
        sensorTaskRunning = false; // This will cause the task to exit its loop and clean up
    }
}

