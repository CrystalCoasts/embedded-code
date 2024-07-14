#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <Preferences.h>



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
#define HALF_MINUTE_US (MINUTE_US / 2)
#define HALF_MINUTE_MS (MINUTE_MS / 2)

//timers for tasks in seconds
// #define SENSOR_TASK_TIMER 30 
// #define WEBSOCKET_TASK_TIMER 0.1
// #define SDCARD_TASK_TIMER 1

//instance declarations
const char* SSID = "seawall";
const char* PASSWD = "12345678";
SdFat32 SD;
Preferences preferences;

//status variables
bool cardMount = false;
bool isConnected = false;

// global variables
const char* JSON_DIR_PATH = "/jsonFiles";
const char* CSV_DIR_PATH = "/csvFiles";

//timers
// volatile uint64_t powerOnTimer = (3600 * 1000) * 2;  // 2 hours
volatile uint64_t powerOnTimer = HALF_MINUTE_US;   // 30 seconds, for sleep and wakeup
volatile uint64_t powerOffTimer = HALF_MINUTE_US;  // 30 seconds, for sleep and wakeup
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
void saveTimerSettings();
void loadTimerSettings();

void setup() {
    setCpuFrequencyMhz(80);
    Serial.begin(115200);
    Wire.begin(); // initialize early to ensure sensors can use it

   loadTimerSettings();

    //setup spi 
    SPI.begin(18, 19, 23, 5);
    SPI.setDataMode(SPI_MODE0);

    // Initialize sensors before wifi
    temp.begin();
    tbdty.begin();
    phGloabl.begin();
    DO.begin();
    sal.begin(); //also tds
    // sal.EnableDisableSingleReading(SAL, 1); // now readX takes care of setting salinity or tds readings


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
    
    // // Register wifi event handlers
    // WiFi.onEvent(onWifiConnect, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    // WiFi.onEvent(onWifiDisconnect, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

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
    // shutdownTimerHandle = xTimerCreate("ShutdownTimer", pdMS_TO_TICKS(powerOffTimer), pdFALSE, (void*) 0, shutdownTimerCallback);
    // xTaskCreate(wifiCheckTask, "WiFi Check", 2048, NULL, 1, NULL);
    // xTimerStart(shutdownTimerHandle, 0);
    startUploadTask();



    String msg = MAIN_TAG + String (" Setup done");
    Serial.println(msg);
}



void loop() {
//empty managed on tasks
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
            vTaskDelay(pdMS_TO_TICKS(10000)); // Delay before next execution cycle
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

// void wifiCheckTask(void *pvParameters) {
//     const TickType_t xFrequency = pdMS_TO_TICKS(5000);  // Check every 5 seconds
//     while (1) {
//         if (WiFi.status() != WL_CONNECTED) {
//             Serial.println("Attempting to reconnect to WiFi...");
//             WiFi.disconnect();
//             WiFi.reconnect();
//             int retryCount = 0;
//             while (WiFi.status() != WL_CONNECTED && retryCount < 10) {
//                 delay(500);
//                 retryCount++;
//             }
//             if (WiFi.status() == WL_CONNECTED) {
//                 Serial.println("Reconnected to WiFi.");
//             } else {
//                 Serial.println("Failed to reconnect to WiFi.");
//             }
//         }
//         vTaskDelay(xFrequency);
//     }
// }



/* SYSTEM CALLBACKS */
// void onWifiConnect(WiFiEvent_t event, WiFiEventInfo_t info) {
//     Serial.println("Connected to WiFi.");
//     Serial.println("IP Address: " + WiFi.localIP().toString());
//     ws.init();
//     Serial.println("starting upload task");
//     startUploadTask();
// }

// Handler when disconnected from WiFi
// void onWifiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info) {
//     Serial.println("Disconnected from WiFi.");
//     ws.stop();
//     Serial.println("stopping upload task");
//     stopUploadTask();
    
    
// }

// void reconnectWiFi() {
//     if (WiFi.status() != WL_CONNECTED) {
//         Serial.println("Attempting to reconnect to WiFi...");
//         WiFi.disconnect(true);  // Disconnect from the WiFi network
//         WiFi.reconnect();       // Attempt to reconnect
//         unsigned long startTime = millis();
//         while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
//             delay(500);
//             Serial.print(".");
//         }
//         if (WiFi.status() == WL_CONNECTED) {
//             Serial.println("\nReconnected to WiFi.");
//         } else {
//             Serial.println("\nFailed to reconnect to WiFi.");
//         }
//     }
// }


// Timer callback function
void shutdownTimerCallback(TimerHandle_t xTimer) {
    Serial.println("Power-off timer expired. Initiating shutdown sequence.");
    powerOffSequence();
}


/* HELPER FUNCTIONS */

void powerOffSequence() {
    // stop all tasks
    // if (WiFi.status() == WL_CONNECTED) {
    //     WiFi.disconnect(); //this takes care of ws and upload task
    // }
    ws.stop();
    stopSensorTask();
    saveTimerSettings();
    // check which wakeup time is closer, update nvs for wakeup
    // deep sleep 
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

void saveTimerSettings() {
    preferences.begin("my_timers", false); // Open NVS in read/write mode
    preferences.putInt("powerOnTimer", powerOnTimer);
    preferences.end(); // Close NVS to save changes
}

void loadTimerSettings() {
    preferences.begin("my_timers", true); // Open NVS in read-only mode
    powerOnTimer = preferences.getInt("powerOnTimer", 30); // Default to 30 if not set
    preferences.end(); // Close NVS after reading
}

