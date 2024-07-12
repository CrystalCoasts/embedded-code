#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <Wire.h>



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

//timers for tasks in seconds
#define SENSOR_TASK_TIMER 30 
#define WEBSOCKET_TASK_TIMER 0.1
#define SDCARD_TASK_TIMER 1

#define I2C_SLAVE_ADDR 0x04  // Set the I2C address for the slave ESP32

//instance declarations
const char* SSID = "seawall";
const char* PASSWD = "12345678";
SdFat32 SD;

//status variables
bool cardMount = false;
bool isConnected = false;

//timers
// volatile uint64_t powerOnTimer = (3600 * 1000) * 2;  // 2 hours
volatile uint64_t powerOnTimer = 30; //inmediate wake up for testing
volatile uint64_t powerOffTimer = 30000;  // 10 mins
TimerHandle_t shutdownTimerHandle;
void shutdownTimerCallback(TimerHandle_t xTimer) ; //callback

// I2C Addresses and Commands
const int slaveAddress = 0x04;  // Example slave address
const byte CMD_SEND_TIME = 0x01;
const byte CMD_SET_POWER_ON_TIMER = 0x02;
const byte CMD_POWER_OFF = 0x03;


//tasks semaphores
SemaphoreHandle_t sdCardMutex;
SemaphoreHandle_t sensorMutex;

//task handlers and functions prototypes
TaskHandle_t TaskSensorHandle = NULL;
void sendI2CCommand(byte command, uint64_t data);
void powerOffSequence();
void sendI2CTimeCommand(byte command, struct tm currentTime)

//task functions
void sensorTask(void *pvParameters);

void setup() {
    setCpuFrequencyMhz(80);
    Serial.begin(115200);

    Wire.begin();
    sendI2CCommand(CMD_SET_POWER_ON_TIMER, powerOnTimer); // early so slave can go to sleep

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
    if(!SD.begin(SdSpiConfig(5, SHARED_SPI, SD_SCK_MHZ(4)))){
        String msg = SD_TAG + String (" Card Mount Failed");
        Serial.println(msg);
    }
    else  {
        String msg = SD_TAG + String (" Card mount sucessful!");
        Serial.println(msg);
        cardMount = true;
    }  
    
    // Initialize WiFi
    WiFi.begin(SSID, PASSWD);
    if (WiFi.status() == WL_CONNECTED){
        String msg = SD_TAG + String (" WiFi connected");
        Serial.println(msg);
        isConnected = true;
    }
    
    //other system initializations
    rtc_begin();
    ws.init();


    //create tasks and setup powerOff timer
    xTaskCreate(sensorTask, "Sensor Task", 2048, NULL, 1, &TaskSensorHandle);
    shutdownTimerHandle = xTimerCreate("ShutdownTimer", pdMS_TO_TICKS(powerOffTimer), pdFALSE, (void*) 0, shutdownTimerCallback);
    xTimerStart(shutdownTimerHandle, 0);

    String msg = MAIN_TAG + String (" Setup done");
    Serial.println(msg);
}



void loop() {
//empty managed on tasks
}

void sensorTask(void *pvParameters) {
    for(;;) {
        Serial.println("[TASKS] Sensor task running");
        SensorData data;
        readSensorData(data);
        // validateSensorReadings(data);
        printDataOnCLI(data);
        // The SD card access is now handled within the saveCSVData function
        if (!saveCSVData(SD, prepareCSVPayload(data))) {
            Serial.println("[TASKS] Failed to save CSV data.");
        }

        Serial.println("[TASKS] Sensor task sleeping");
        vTaskDelay(pdMS_TO_TICKS(SENSOR_TASK_TIMER * 1000));  // Delay the task for SENSOR_TASK_TIMER seconds
    }
}

//comms tasks
void sendI2CCommand(byte command, uint64_t data) {
    Wire.beginTransmission(slaveAddress);
    Wire.write(command);
    byte* dataBytes = (byte*)&data;
    for (int i = 0; i < sizeof(data); i++) {
        Wire.write(dataBytes[i]); // send each byte of the data
    }
    byte error = Wire.endTransmission();
    if (error != 0) {
        Serial.print("I2C transmission error: "); Serial.println(error);
    }
}

void sendI2CTimeCommand(byte command, struct tm currentTime) {
    Wire.beginTransmission(slaveAddress);
    Wire.write(command);
    Wire.write((byte*)&currentTime, sizeof(currentTime));  // Send the whole tm structure
    Wire.endTransmission();
}

void powerOffSequence() {
    struct tm currentTime = get_current_time();  // Get the current local time as a tm struct
    sendI2CTimeCommand(CMD_SEND_TIME, currentTime);  // Send current time as a struct tm
    sendI2CCommand(CMD_SET_POWER_ON_TIMER, powerOnTimer);  // Send the power on timer
    sendI2CCommand(CMD_POWER_OFF, 0);  // Command to power off
}

// Timer callback function
void shutdownTimerCallback(TimerHandle_t xTimer) {
    Serial.println("Power-off timer expired. Initiating shutdown sequence.");
    powerOffSequence();
}