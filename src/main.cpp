#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <cstdint>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <Preferences.h>
#include <esp_sleep.h>

#include <esp_task_wdt.h>
#include <string>

#include "HardwareSerial.h"
#include "globals.h"

// Sensor headers
#include "DOSensor.h"
#include "TempSensor.h"
#include "TurbiditySensor.h"
#include "SalinitySensor.h"
//#include "pHSensor.h"
#include "AtlasTemp.h"
#include "AtlasPH.h"
#include "Cellular.h"
#include "ioExtender.h"

//helpers
#include "io_handler.h" //this includes SdFat32
#include "rtc_handler.h" // includes ntp related headers
//#include "websockets.h"

// error tags
#define SD_TAG "[SD_CARD]"
#define WIFI_TAG "[WIFI]"
#define SENSOR_TAG "[SENSORS]"
#define MAIN_TAG "[MAIN]"
#define SIM_TAG "[SIM_CARD]"

// Time units in milliseconds (for tasks)
#define QUARTER_MINUTE_MS (MINUTE_MS / 4)
#define HALF_MINUTE_US (MINUTE_US / 2)
#define QUARTER_MINUTE_US (MINUTE_US / 4)
#define HALF_MINUTE_MS (MINUTE_MS / 2)
#define BATTERY_PIN 27

// ESP32 LilyGO T-SIM7000G SD Card Pins
#define SD_MISO     2
#define SD_MOSI     15
#define SD_SCLK     14
#define SD_CS       13

//instance declarations
const char* SSID = "seawall";
const char* PASSWD = "12345678";

// status variables
bool cardMount = false;
bool isConnected = false;

// global variables
const char* JSON_DIR_PATH = "/jsonFiles";
const char* CSV_DIR_PATH = "/csvFiles";

// battery values
const uint16_t BATTERY_CHARGE = 10000; // 10000 mAh
const uint8_t BATTERY_DRAW_SLEEP = 60; // 60 mA
const uint8_t BATTERY_DRAW_ACTIVE = 220; // 190 mA

bool sdON = false;

// Global variables for battery management
Preferences prefs;
volatile uint16_t batteryLevel = BATTERY_CHARGE ; // Default battery level
unsigned long lastUpdateTime = 0;

#ifndef POWER_ON_TIMER
#define POWER_ON_TIMER "3"
#endif
#ifndef POWER_OFF_TIMER
#define POWER_OFF_TIMER "3"
#endif
#ifndef READ_RATE
#define READ_RATE "30"
#endif

// timers
// volatile uint64_t powerOnTimer = (3600 * 1000) * 2;  // 2 hours
uint64_t SYSTEM_POWER_ON = std::stoi(POWER_ON_TIMER) * MINUTE_US;   //powers on after 25 minutes
volatile uint64_t USER_POWER_ON = 5 * HOUR_US;

uint64_t SYSTEM_POWER_OFF = std::stoi(POWER_OFF_TIMER) * MINUTE_MS;  // powers off after 5 minutes
const uint64_t SENSOR_TASK_TIMER =  std::stoi(READ_RATE) * SECOND_MS;  //HALF_MINUTE_MS; // 30 seconds, for tasks

//tasks semaphores
SemaphoreHandle_t sdCardMutex;
SemaphoreHandle_t sensorMutex;
SemaphoreHandle_t simCardMutex;

//handlers and running status
TimerHandle_t shutdownTimerHandle;
TaskHandle_t TaskSensorHandle = NULL;
TaskHandle_t TaskUploadDataHandle = NULL;
TaskHandle_t TaskReadBatteryHandle = NULL;
bool sensorTaskRunning = false;
bool webSocketTaskRunning = false;
bool uploadDataTaskRunning= false;
bool batteryTaskRunning = false;

bool dataSaved = false; // marked when the sensors successfully read and saved data (used to start attempting uploads)

//task functions and callbacks
void sensorTask(void *pvParameters);
void uploadTask(void *pvParameters);
void readBatteryTask(void *pvParameters);

void shutdownTimerCallback(TimerHandle_t xTimer) ; //callback


//helper functions
void startUploadTask();
void stopUploadTask();
void stopSensorTask();
void powerOffSequence();

void setup() {
    //setCpuFrequencyMhz(80);                             //Sets cpu frequency to 80 Mhz to save 20% power
    Serial.begin(115200);
    //sensors.begin();

    Wire.begin(21,22);
    // Initialize SD card
    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);        //SD config

    if(!SD.begin(SD_CS, SPI, 4000000))    { 
        Serial.println("SD Mount failed!");
    }else{
        Serial.println("SD Mount successful!");
    }
    
    #ifndef CELLULAR
        // Wifi init
        Wire.begin(); // initialize early to ensure sensors can use it
        Initialize WiFi we need to continue even if wifi fails
        WiFi.begin(SSID, PASSWD);
        if (WiFi.status() == WL_CONNECTED){
            String msg = SD_TAG + String (" WiFi connected");
            Serial.println(msg);
            isConnected = true;
        }
    #else
        //Cellular init
        sim.begin();
        if(sim.isGprsConnected())  {
            String msg = SD_TAG + String ("Cellular connected");
            Serial.println(msg);
            isConnected = true;
        }
    #endif
    
    // Initialize all peripherals
    mcpGlobal.begin();
    i2cadc.begin();
    temp.begin();
    tbdty.begin();

    //phGloabl.begin();   
    DO.begin();
    sal.begin(); //also tds & ec
    atlasPHSensor.begin();
    atlasTempSensor.begin();

    // Create mutexes
    sdCardMutex = xSemaphoreCreateMutex();
    sensorMutex = xSemaphoreCreateMutex();
    simCardMutex = xSemaphoreCreateMutex();
  
    // Initialize NVS for battery level
    prefs.begin("battery_storage");
    batteryLevel = prefs.getUInt("batteryLevel", BATTERY_CHARGE); // Default to full charge if not set
    prefs.end();
    rtc_begin();
    //ws.init();

    //create tasks and setup powerOff timer
    lastUpdateTime = millis(); // Set initial time for battery updates
    xTaskCreate(readBatteryTask, "Battery Task", 2048, NULL, 1, &TaskReadBatteryHandle);
    xTaskCreate(sensorTask, "Sensor Task", 8192, NULL, 1, &TaskSensorHandle);
    shutdownTimerHandle = xTimerCreate("ShutdownTimer", pdMS_TO_TICKS(SYSTEM_POWER_OFF), pdFALSE, (void*) 0, shutdownTimerCallback);
    xTimerStart(shutdownTimerHandle, 0);
    startUploadTask();

    String msg = MAIN_TAG + String (" Setup done");
    Serial.println(msg);
}



void loop() {
}

/* TASKS */
void sensorTask(void *pvParameters) {
    struct tm timeinfo;
    bool timeObtained = false;
    while (!timeObtained) { // tries to obtain time in loop to retry if it fails
        if (xSemaphoreTake(simCardMutex, pdMS_TO_TICKS(5000))) { // acquire simCardMutex to get time
            if(getCurrentTime(timeinfo)){ // tries to getCurrentTime
                timeinfo = get_current_time(); // sets the current time to timeinfo to be used for saving sensor reads
                Serial.println("[TASKS] Successfully obtained time for sensor reads");
                xSemaphoreGive(simCardMutex);
                timeObtained = true;
            } else {
                Serial.println("[TASKS] sensorTask: failed to getCurrentTime for sensor reads");
                vTaskDelay(pdMS_TO_TICKS(5000));
            }
        } else {
            Serial.println("[TASKS] sensorTask: failed to take sim mutex to get time");
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }

    sensorTaskRunning = true;                                           // Used to stop task later when sleeping
    //read the sensor data
    SensorData data;                                            //Initializes sensor data structure
    readSensorData(data);                                       // Reads data
    printDataOnCLI(data);                                       // Prints onto terminal
    bool csvDataSaved = false;
    bool jsonDataSaved = false;
    // begin saving sensor data
    while (sensorTaskRunning) {
        Serial.println("[TASKS] Sensor task running");
        // attempt to get sd card mutex
        if (xSemaphoreTake(sdCardMutex, pdMS_TO_TICKS(5000))) {
            // attempt to save csv data if it wasn't already saved
            if(!csvDataSaved && saveCSVData(SD, prepareCSVPayload(data), timeinfo)) {
                csvDataSaved = true;
            } else if(!csvDataSaved){
                Serial.println("[TASKS] Failed to save CSV data.");
            }
            // attempt to save json data if it already wasn't saved
            if(!jsonDataSaved && saveJsonData(SD, prepareJsonPayload(data), timeinfo)) {
                jsonDataSaved = true;
            } else if(!jsonDataSaved){
                Serial.println("[TASKS] Failed to save JSON data.");
            }

            // check if all data is saved
            if (jsonDataSaved && csvDataSaved) {
                sensorTaskRunning = false;
                Serial.println("[TASKS] Successfully saved all data!");
                xSemaphoreGive(sdCardMutex);
                dataSaved = true;
            } else {
                vTaskDelay(pdMS_TO_TICKS(1000));
                xSemaphoreGive(sdCardMutex); // give sdCardMutex so that it can take it again (needs rewrite)
            }
        } else {
            Serial.println("[TASKS] sensorTask: failed to take sd mutex to save data");
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }
}
/*
void sensorTask1(void *pvParameters) {

    sensorTaskRunning = true;                                           // Used to stop task later when sleeping
    while (sensorTaskRunning) {
            Serial.println("[TASKS] Sensor task running");
            SensorData data;                                            //Initializes sensor data structure
            readSensorData(data);                                       // Reads data
            printDataOnCLI(data);                                       // Prints onto terminal
            
            // Data saving to SD
            Serial.println("Starting CSV task");                        //saves to csv
            if (!saveCSVData(SD, prepareCSVPayload(data))) {
                Serial.println("[TASKS] Failed to save CSV data.");
                cardMount = false;
            }

            Serial.println("Starting JSON task");                       //saves to json
            if (!saveJsonData(SD, prepareJsonPayload(data))) {
                Serial.println("[TASKS] Failed to save JSON data.");
            }

            Serial.println("[TASKS] Sensor task sleeping");
            vTaskDelay(pdMS_TO_TICKS(SENSOR_TASK_TIMER));               // Delay the task for SENSOR_TASK_TIMER seconds

    }

    // Clean up or prepare to delete task
    sensorTaskRunning = false;
    vTaskDelay(pdMS_TO_TICKS(10000));
    vTaskDelete(NULL);                                                  // Optionally delete the task explicitly
}
*/

void uploadTask(void *pvParameters) {
    for (;;) {
        // checks if data was saved by sensors since startup, 
        // if not continue until it has been saved to avoid sensor never getting a read successfully
        if(!dataSaved) {
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }
        #ifndef CELLULAR
            // attempts to connect to wifi to send data
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("WiFi not connected. Skipping upload.");
                vTaskDelay(pdMS_TO_TICKS(5000)); // Delay before next execution cycle
                continue;
            }
        #else
            if(!sim.isGprsConnected()){                                  // attempts to connect to the cellular network to send data
                    Serial.println("Cellular not connected.");
                    sim.gprsConnect();
                    vTaskDelay(pdMS_TO_TICKS(5000));                     // Delay before next execution cycle
                    continue;
            }

        #endif

        // 1. Grab the SD Mutex FIRST (Matching save functions)
        if (xSemaphoreTake(sdCardMutex, pdMS_TO_TICKS(5000))) {
            
            // 2. NOW it is safe to interact with the SD card
            File root, file;
            if (!(root = SD.open(JSON_DIR_PATH, FILE_READ))) {         // Attempts to open json file
                Serial.println("Failed to open directory");
                xSemaphoreGive(sdCardMutex); // CRITICAL: Give it back before retrying
                vTaskDelay(pdMS_TO_TICKS(10000));            // Wait for 10 seconds before retrying
                continue;
            }

            String fileName;
            
            // 3. Grab the SIM Mutex SECOND
            if (xSemaphoreTake(simCardMutex, pdMS_TO_TICKS(5000))) { 
                
                // loop through all files in directory and attempt to upload
                while ((file = root.openNextFile())) {
                    if (file.isDirectory()) {
                        file.close();
                        continue;
                    }

                    fileName = file.name();
                    if (String(fileName).startsWith(".") || !String(fileName).endsWith(".json")) {  // If file isn't a json file, deletes the file.
                        file.close();
                        SD.remove(fileName);
                        continue;
                    }

                    uploadDataTaskRunning = true;
                    bool allLinesUploaded = true;
                    String jsonLine;
                    char ch;
                    while (file.available()) {                          // For every line in the json uploads all the data in the file
                        ch = file.read();
                        if (ch == '\n' || !file.available()) {          // sends data for every json line
                            if (!jsonLine.isEmpty()) {
                                jsonLine.trim();
                                if (!uploadData(jsonLine)) {
                                    Serial.println("Failed to upload: " + jsonLine);
                                    allLinesUploaded = false;
                                    uploadDataTaskRunning = false;
                                    break;
                                }
                                vTaskDelay(pdMS_TO_TICKS(2000));
                                jsonLine = "";                          // Reset the line buffer
                            }
                        } else {
                            jsonLine += ch;
                        }
                    }
                    sim.serverDisconnect();
                    uploadDataTaskRunning = false;
                    file.close();

                    if (allLinesUploaded) {                             //if all lines in the file were upload, removes file from SD
                        SD.remove(String(JSON_DIR_PATH) + "/" + String(fileName)); // Ensure the path is correct
                        Serial.println(String(fileName) + " uploaded and deleted successfully.");
                        sim.connected = false;
                    } else {    
                        Serial.println("Not all lines in the file were uploaded successfully.");
                    }
                }
                
                // 4. Loop execution complete, Give both mutexes back in reverse order to Taking them
                xSemaphoreGive(simCardMutex);
                xSemaphoreGive(sdCardMutex);
                
            } else {
                Serial.println("Couldn't get Sim mutex");
                // CRITICAL: Give back the SD mutex since failed to get SIM mutex
                xSemaphoreGive(sdCardMutex);
            }
            
            root.close();
            
        } else {
            Serial.println("Couldn't get SD mutex");
        }
        vTaskDelay(pdMS_TO_TICKS(10000));                           // Delay before next execution cycle
    }
        
}



/* SYSTEM CALLBACKS */
// Timer callback function
void shutdownTimerCallback(TimerHandle_t xTimer) {
    Serial.println("Power-off timer expired. Checking current time.");
    // CRITICAL: Do not kill the power if an upload is currently in progress
    if (uploadDataTaskRunning) {
        Serial.println("Upload in progress! Delaying shutdown by 15 seconds...");
        xTimerChangePeriod(shutdownTimerHandle, pdMS_TO_TICKS(15000), 0);
        return; // Exit the callback without powering down
    }
    struct tm timeinfo = get_current_time();

    #ifndef CELLULAR
        // if (WiFi.status() == WL_CONNECTED) {
        //     // If connected, reset the timer
        //     Serial.println("Active WiFi connection detected. Resetting power-off timer.");
        //     xTimerReset(shutdownTimerHandle, 0);  // Reset the timer to delay shutdown
        // } else {
        //     // No active connection, proceed to power off
        //     Serial.println("No active connection. Initiating shutdown sequence.");
        //     powerOffSequence();
        // }
    #else
        if(timeinfo.tm_hour < 6 || timeinfo.tm_hour > 19)       //checks if time if before 6AM or more than 7PM
            SYSTEM_POWER_ON = 55 * MINUTE_US;                   //sets poweroff timer to wak up once an hour
        //else
        //    SYSTEM_POWER_ON = 3 * MINUTE_US;                   //sets poweroff timer to wake up twice an hour

        Serial.println("Power-off timer expired. Executing power down for" + String((float)(SYSTEM_POWER_ON/(60000000))));

        powerOffSequence();
    #endif
}


/* HELPER FUNCTIONS */
void powerOffSequence() {
    Serial.println("Powering off...");

    // Load settings, stop tasks, and prepare for shutdown
    for(int i = 0; i <7; i++)   {
        mcpGlobal.digitalWriteA(i, LOW);                        // sets gpio extender IO to low
    }
    prefs.begin("my_timers", false);
    prefs.end();
    loadTimerSettings();
    stopSensorTask();
    stopUploadTask();
    //ws.stop();

    // Calculate next wakeup time and adjust USER_POWER_ON
    uint64_t power_on = (USER_POWER_ON < SYSTEM_POWER_ON) ? USER_POWER_ON : SYSTEM_POWER_ON;
    USER_POWER_ON = (USER_POWER_ON > SYSTEM_POWER_ON) ? (USER_POWER_ON - SYSTEM_POWER_ON) : UINT64_MAX;
    
    // Set the wakeup timer before calculating sleep battery consumption
    esp_sleep_enable_timer_wakeup(power_on);

    // Calculate the battery consumed during sleep based on the power_on time
    unsigned long sleepTimeMs = power_on / 1000; // Convert from microseconds to milliseconds
    uint16_t sleepConsumption = (BATTERY_DRAW_SLEEP * sleepTimeMs / 3600000.0); // Convert ms to hours
    batteryLevel = max(0, batteryLevel - sleepConsumption);

    // Save the updated battery level to NVS
    prefs.begin("battery_storage", false);
    prefs.putUInt("batteryLevel", batteryLevel);
    prefs.end();

    // Save the timer settings to NVS using rtc_handler's namespace
    saveTimerSettings(USER_POWER_ON);
    Serial.println("Entering deep sleep...");
    esp_deep_sleep_start();
}

void startUploadTask() {
    if (TaskUploadDataHandle == NULL) {
        xTaskCreatePinnedToCore(uploadTask, "UploadData", 8192, NULL, 1, &TaskUploadDataHandle,1);
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

void readBatteryTask(void *pvParameters) {
    const uint64_t updateIntervalMs = 60000; // 60 seconds
    TickType_t xLastWakeTime = xTaskGetTickCount();
    lastUpdateTime = millis(); 

    while (true) {
        unsigned long currentTime = millis();
        unsigned long elapsedTime = currentTime - lastUpdateTime;                       // Calculate time since last update

        if (elapsedTime >= updateIntervalMs) {
            // Calculate power consumption during active time
            uint16_t powerConsumed = (BATTERY_DRAW_ACTIVE * elapsedTime / 3600000.0);   // Convert ms to hours and calculate
            batteryLevel = max(0, batteryLevel - powerConsumed);

            prefs.begin("battery_storage", false);
            prefs.putUInt("batteryLevel", batteryLevel);
            prefs.end();

            // Calculate battery percentage for display or reporting
            int batteryPercentage = (batteryLevel * 100) / BATTERY_CHARGE;
            lastUpdateTime = currentTime;
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(updateIntervalMs));
    }
}
