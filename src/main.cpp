#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <Preferences.h>
#include <esp_sleep.h>

#include "soc/rtc_cntl_reg.h"
#include "soc/rtc.h"
#include "driver/rtc_io.h"
#include "driver/i2c.h"



// Sensor headers
#include "TempSensor.h"
#include "TurbiditySensor.h"
#include "SalinitySensor.h"
#include "pHSensor.h"
#include "ioExtender.h"
#include "Adafruit_MCP23X17.h"

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

SdFat32 SD;

//status variables
bool cardMount = false;
bool isConnected = false;

// global variables
const char* JSON_DIR_PATH = "/jsonFiles";
const char* CSV_DIR_PATH = "/csvFiles";

//battery values
const uint16_t BATTERY_CHARGE = 10000; // 10000 mAh
const uint8_t BATTERY_DRAW_SLEEP = 60; // 60 mA
const uint8_t BATTERY_DRAW_ACTIVE = 190; // 190 mA

bool sdON = false;

// Global variables for battery management
Preferences prefs;
volatile uint16_t batteryLevel = BATTERY_CHARGE ; // Default battery level
unsigned long lastUpdateTime = 0;


//timers
// volatile uint64_t powerOnTimer = (3600 * 1000) * 2;  // 2 hours
const uint64_t SYSTEM_POWER_ON = 25 * MINUTE_US;
volatile uint64_t USER_POWER_ON = 5 * HOUR_US;

uint64_t SYSTEM_POWER_OFF = 30 * MINUTE_MS;  
const uint64_t SENSOR_TASK_TIMER =  5000;  //HALF_MINUTE_MS; // 30 seconds, for tasks

//tasks semaphores
SemaphoreHandle_t sdCardMutex;
SemaphoreHandle_t sensorMutex;

//handlers and running status
TimerHandle_t shutdownTimerHandle;
TaskHandle_t TaskSensorHandle = NULL;
TaskHandle_t TaskUploadDataHandle = NULL;
TaskHandle_t TaskReadBatteryHandle = NULL;
bool sensorTaskRunning = false;
bool webSocketTaskRunning = false;
bool uploadDataTaskRunning= false;
bool batteryTaskRunning = false;

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

// OneWire oneWire(41);
// DallasTemperature sensors(&oneWire);
Adafruit_MCP23X17 mcp2;

void setup() {
    //setCpuFrequencyMhz(80);
    Serial.begin(115200);
    //sensors.begin();

    Wire.begin(15, 16);
    //Wire.begin(); // initialize early to ensure sensors can use it
    // Initialize WiFi we need to continue even if wifi fails
    WiFi.begin(SSID, PASSWD);
    if (WiFi.status() == WL_CONNECTED){
        String msg = SD_TAG + String (" WiFi connected");
        Serial.println(msg);
        isConnected = true;
    }

    int i2c_master_port = 0;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 15,         // select GPIO specific to your project
        .scl_io_num = 16,         // select GPIO specific to your project
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .clk_flags = 0,                          // you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here
    };
    i2c_driver_install(I2C_NUM_MAX, I2C_MODE_MASTER, 500, 500, 0);

    //setup spi 
    // SPI.begin(18, 19, 23, 5);
    // SPI.setDataMode(SPI_MODE0);

    // Initialize sensors before wifi

    //mcp.begin_I2C();
    temp.begin();
    tbdty.begin();
    phGloabl.begin();
    DO.begin();
    sal.begin(); //also tds & ec
    pinMode(40,OUTPUT);
    digitalWrite(40,HIGH);
    pinMode(38,OUTPUT);
    digitalWrite(38,HIGH);
    delay(200);
    // mcpGlobal.begin();
    // mcpGlobal.pinMode(0,OUTPUT);
    // mcpGlobal.digitalWrite(0,HIGH);
    mcp2.begin_I2C();
    mcp2.pinMode(8,OUTPUT);
    mcp2.digitalWrite(8,HIGH);
  

    // digitalWrite(40,LOW);
    // delay(100);
    // digitalWrite(40,HIGH);

    // uncomment appropriate mcp.begin
    // if (!mcp.begin_I2C()) {
    // //if (!mcp.begin_SPI(CS_PIN)) {
    //     Serial.println("Error.");
    // }

    // // configure pin for output
    // mcp.pinMode(0, OUTPUT);
    // mcp.digitalWrite(0,HIGH);

    //sal.calibrate();

    // Create mutexes
    sdCardMutex = xSemaphoreCreateMutex();
    sensorMutex = xSemaphoreCreateMutex();
    
    // Initialize SD card

    SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0);
    if (!SD_MMC.begin("/sdcard", true, true))
    {
        String msg = SD_TAG + String (" Card Mount Failed");
        Serial.println(msg);
    }
    uint8_t cardType = SD_MMC.cardType();

        if (cardType == CARD_NONE)
    {
        Serial.println("No SD_MMC card attached");
    }

    Serial.print("SD_MMC Card Type: ");
    if (cardType == CARD_MMC)
    {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println("SDHC");
    }
    else
    {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);

    
    // sensors.requestTemperatures(); 
    // Serial.println("Temp:" + int(sensors.getTempCByIndex(0)));


    // if(!SD.begin(SdSpiConfig(5, SHARED_SPI, SD_SCK_MHZ(16)))){
    //     String msg = SD_TAG + String (" Card Mount Failed");
    //     Serial.println(msg);
    //     cardMount = false;
    // }
    // else  {
    //     String msg = SD_TAG + String (" Card mount sucessful!");
    //     Serial.println(msg);
    //     cardMount = true;
    // }  
    
  
    //other system initializations

    // Initialize NVS for battery level
    prefs.begin("battery_storage");
    batteryLevel = prefs.getUInt("batteryLevel", BATTERY_CHARGE); // Default to full charge if not set
    prefs.end();
    rtc_begin();
    ws.init();

    //create tasks and setup powerOff timer
    lastUpdateTime = millis(); // Set initial time for battery updates
    xTaskCreate(readBatteryTask, "Battery Task", 2048, NULL, 1, &TaskReadBatteryHandle);
    xTaskCreate(sensorTask, "Sensor Task", 8192, NULL, 1, &TaskSensorHandle);
    shutdownTimerHandle = xTimerCreate("ShutdownTimer", pdMS_TO_TICKS(SYSTEM_POWER_OFF), pdFALSE, (void*) 0, shutdownTimerCallback);
    xTimerStart(shutdownTimerHandle, 0);
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
        
        // if(!cardMount)  {
        //     SD_MMC.begin("/sdcard", true, true);
        //     String msg = SD_TAG + String (" Card Mount Failed");
        //     Serial.println(msg);
        // }
        // else  {
        //     cardMount = true;
        // }  

        if (!saveCSVData(SD_MMC, prepareCSVPayload(data))) {
            Serial.println("[TASKS] Failed to save CSV data.");
            cardMount = false;
        }
        if (!saveJsonData(SD_MMC, prepareJsonPayload(data))) {
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

        File root, file;
        if (!(root = SD_MMC.open(JSON_DIR_PATH, FILE_READ))) {
            Serial.println("Failed to open directory");
            vTaskDelay(pdMS_TO_TICKS(10000)); // Wait for 10 seconds before retrying
            continue;
        }

        String fileName;
        while (file = root.openNextFile()) { // openNext(&root, O_READ)) { 
            if (file.isDirectory()) {
                file.close();
                continue;
            }

            fileName = file.name();
            if (String(fileName).startsWith(".") || !String(fileName).endsWith(".json")) {
                file.close();
                SD_MMC.remove(fileName);
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
                SD_MMC.remove(String(JSON_DIR_PATH) + "/" + String(fileName)); // Ensure the path is correct
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
    digitalWrite(38, LOW);
    gpio_hold_en(GPIO_NUM_38);
}

void readBatteryTask(void *pvParameters) {
    const uint64_t updateIntervalMs = 60000; // 60 seconds
    TickType_t xLastWakeTime = xTaskGetTickCount();
    lastUpdateTime = millis(); 

    while (true) {
        unsigned long currentTime = millis();
        unsigned long elapsedTime = currentTime - lastUpdateTime; // Calculate time since last update

        if (elapsedTime >= updateIntervalMs) {
            // Calculate power consumption during active time
            uint16_t powerConsumed = (BATTERY_DRAW_ACTIVE * elapsedTime / 3600000.0); // Convert ms to hours and calculate
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
