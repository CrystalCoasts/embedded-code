// #include <Arduino.h>
// #include <WiFi.h>
// #include <HTTPClient.h>
// #include "SPIFFS.h"
// #include <ArduinoJson.h>
// #include "SPI.h"
// #include "FS.h"
// #include "SdFat.h"
// #include "NTPClient.h"
// #include "WiFiUdp.h"
// #include "esp_sntp.h"

// // Sensor headers
// #include "TempSensor.h"
// #include "TurbiditySensor.h"
// #include "SalinitySensor.h"
// #include "pHSensor.h"
// #include <base_surveyor.h>
// #include "DOSensor.h"

// #include "io_handler.h"
// #include "rtc_handler.h"

// #define SLEEP_TIME_US 30000000 // 1 minute = 60000000 - multiply by any number for amount of minutes

// SdFat32 SD;

// const char* SSID = "seawall";
// const char* PASSWD = "12345678";
// const char* WEB_APP_URL = "https://script.google.com/macros/s/AKfycbzdREDYLRb1ew8CjwGY_WnrIU0UWW0Sn3Wr4XdT8Jv0VjXuQxJV7GVCKZeYtEb2zrKb/exec";
// const char *serverName = "https://smart-seawall-server-4c5cb6fd8f61.herokuapp.com/";
// //const char *serverName = "MONGODB_URI=mongodb+srv://lisettehawkins09:cxO0hBBXellzkuAX@cluster0-sensordatassam.sk9l59s.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0-sensorDatasSample";

// const int STATUS_LED_PIN = 2; // Commonly the onboard LED pin on ESP32
// const int BUTTON_PIN = 0;


// Surveyor_pH pH = Surveyor_pH(35);



// void uploadData(String data);

// void sdBegin();

// // Status functions
// void blinkLED(int delayTime);
// void setLEDSolid(bool on);
// bool cardMount = false;

// //Sleep status for night mode, regular mode, and within 5-10 minutes after supposed connection
// enum sleepStatus    {
//     regular = 1,    
//     withinInterval,
//     nightMode,
//     connectedToDevice
// };


// void setup() {

//     setCpuFrequencyMhz(80);
//     Serial.begin(115200);
//     Wire.begin();
//     // Initialize SPIFFS
//     if (!SPIFFS.begin(true)) {
//         Serial.println("An Error has occurred while mounting SPIFFS");
//         return;
//     } else {
//         Serial.println("SPIFFS mounted successfully.");
//     }

//     // pinMode(STATUS_LED_PIN, OUTPUT);
//     // pinMode(BUTTON_PIN, INPUT);

//     // Initialize sensors
//     temp.begin();
//     tbdty.begin();
//     //tbdty.calibrate();
//     sal.begin();
//     sal.EnableDisableSingleReading(SAL, 1);
//     //sal.EnableDisableSingleReading(TDS,1);
//     DO.begin();
//     phGloabl.begin();

//     SPI.begin(18, 19, 23, 5);
//     SPI.setDataMode(SPI_MODE0);
//     if(!SD.begin(SdSpiConfig(5, SHARED_SPI, SD_SCK_MHZ(16)))){
//         Serial.println("Card Mount Failed");
        
//     }else   {
//         Serial.println("Card mount sucessful!");
//         cardMount = true;
//     } 

//     // Initialize WiFi
//     WiFi.begin(SSID, PASSWD);

//     rtc_begin();




// }

// void loop() {
//     if(cardMount != true)   {
//         if(!SD.begin(SdSpiConfig(5, SHARED_SPI, SD_SCK_MHZ(16)))){
//             Serial.println("Card Mount Failed");
//         }else   {
//             Serial.println("Card mount sucessful!");
//             cardMount = true;
//         }   
//     }
   
//     // Sensor data
//     SensorData data;   
//     readSensorData(data);

//     // Default values for other sensors
//     data.tds = 111.0;
//     data.pHValid = true;

//     // Validate readings
//     //validateSensorReadings(data);

//     String jsonPayload = prepareJsonPayload(data);
//     String csvPayLoad = prepareCSVPayload(data);
//     printDataOnCLI(data);


//     jsonPayload= prepareJsonPayload(data);
//     csvPayLoad = prepareCSVPayload(data);
//     saveDataToJSONFile(SD, jsonPayload);
//     saveCSVData(SD, csvPayLoad);

//     printLocalTime();

// }


// void uploadData(String jsonData) {
//     if (WiFi.status() == WL_CONNECTED) {
//         Serial.println("Sending to Google.");
//         // Serial.print(jsonData);
//         HTTPClient http;
//         http.begin(WEB_APP_URL);
//         int httpResponseCode = http.POST(jsonData);
//         http.addHeader("Content-Type", "application/json");
//         http.end();
//     } else {
//         Serial.println("WiFi is not connected. Skipping data upload.");
//     }
// }


// void blinkLED(int delayTime) {
//     digitalWrite(STATUS_LED_PIN, HIGH);
//     delay(delayTime);
//     digitalWrite(STATUS_LED_PIN, LOW);
//     delay(delayTime);
// }

// void setLEDSolid(bool on) {
//     digitalWrite(STATUS_LED_PIN, on ? HIGH : LOW);
// }

