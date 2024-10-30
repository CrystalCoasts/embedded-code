// #include <Arduino.h>
// #include <esp_adc_cal.h>
// #include <esp32-hal-adc.h>

// void setup() 
// {
//   Serial.begin(115200);
//   analogReadResolution(12);
//   pinMode(1, INPUT);  //GPIO4
//   adcAttachPin(1);    //GPIO4

// }

// void loop() 
// {
//   int RawADC= adc1_get_raw(ADC1_CHANNEL_0); //GPIO9
//   Serial.println(RawADC);
//   delay(100);
// }

// #include <Arduino.h>

// //required
// #include <WiFi.h>
// #include <HTTPClient.h>
// #include "FS.h"
// #include "SPIFFS.h" //SPI Flash File System -- basically on board memory
// #include "esp_sleep.h"
// #include <ArduinoJson.h>

// #include <iostream>
// #include <cmath>

// //our classes
// #include "TempSensor.h"
// #include "TurbiditySensor.h"
// #include "SalinitySensor.h"
// // #include "config.h"

// const char* SSID = "seawall";
// const char* PASSWD = "12345678";
// const char* WEB_APP_URL = "https://script.google.com/macros/s/AKfycbzdREDYLRb1ew8CjwGY_WnrIU0UWW0Sn3Wr4XdT8Jv0VjXuQxJV7GVCKZeYtEb2zrKb/exec";

// const int STATUS_LED_PIN = 2; // Commonly the onboard LED pin on ESP32
// const int BUTTON_PIN = 0;

// // Forward declarations
// void saveDataToCSV(String data);
// void uploadData(String data);
// void SensorTask(void *pvParameters);
// void dataTransmissionTask(void *pvParameters);
// String prepareJsonPayload(float pH, float oxygenLevel, float salinity, float turbidity, float tds, float temperature);
// String readDataFromCSV();
// bool validateSensorReadings(float& humidity, float& temperature, float& turbidity, float& salinity, float& tds, float& ph, float& oxygenLevel);
// void printStackSpace();
// void printDataFromCSV();


// //status functions
// void blinkLED(int delayTime);
// void setLEDSolid(bool on) ;

// void setup() {
//     // General setup
//     Serial.begin(115200);
//     Wire.begin();

//     // Initialize SPIFFS
//     if (!SPIFFS.begin(true)) {
//         Serial.println("An Error has occurred while mounting SPIFFS");
//         return; // Stop further execution if SPIFFS fails to initialize
//     } else {
//         Serial.println("SPIFFS mounted successfully.");
//     }

//     TaskHandle_t sensorTaskHandle = NULL;
//     TaskHandle_t dataTransmissionTaskHandle = NULL;
//     pinMode(STATUS_LED_PIN, OUTPUT);

//     // Initialize sensors
//     temp.begin();
//     tbdty.begin();
//     sal.begin();
//     sal.EnableDisableSingleReading(SAL,1);

//     // Initialize WiFi
//     WiFi.begin(SSID, PASSWD);

//     xTaskCreate(SensorTask, "Sensor Task", 8192, NULL, 1, &sensorTaskHandle);
//     xTaskCreate(dataTransmissionTask, "Data Transmission Task", 8192, NULL, 1, &dataTransmissionTaskHandle); // Same here

//     // // Print stack space periodically
//     // xTaskCreate([](void *pvParameters) {
//     //     while (1) {
//     //         printStackSpace(); // Call your stack space function
//     //         vTaskDelay(pdMS_TO_TICKS(10000)); // Every 10 seconds
//     //     }
//     // }, "StackMonitor", 2048, NULL, 1, NULL);
// }

// void SensorTask(void *pvParameters) {
//     while (1) {
//         // value = round(value*1000)/1000 -> round to 2 decimal places
//         float humidity,salinity, turbidity, waterTemp;

//         temp.readHumidity(&humidity);
//         temp.readTemperature(FAHRENHEIT,&waterTemp);
//         tbdty.readTurbidity(&turbidity);
//         sal.readSalinity(&salinity);
//         // float humidity = 96.5;
//         // float waterTemp = 76.0;
//         // float turbidity = 3000;
        
//         //round reads:
//         humidity = round(humidity * 1000.0) / 1000.0;
//         waterTemp = round(waterTemp * 1000.0) / 1000.0;
//         turbidity = round(turbidity * 1000.0) / 1000.0;
//         salinity = round(salinity * 1000.0) / 1000.0;

        
       
//         // default values until we get the sensors
        
//         float tds = 111.0;
//         float pH = 7.0;
//         float oxygenLevel = 36.0;
//         // String values = String(humidity, 3) + "," + 
//         //                 String(waterTemp, 3) + "," + 
//         //                 String(turbidity, 3) + "," + 
//         //                 String(salinity, 3) + "," +
//         //                 String(tds, 3) + "," + 
//         //                 String(pH, 3) + "," + 
//         //                 String(oxygenLevel, 3) ;
//         bool validReading =validateSensorReadings(humidity, waterTemp, turbidity, salinity, tds, pH, oxygenLevel);
//         // Serial.println(values);

//         String jsonPayload = prepareJsonPayload(pH, oxygenLevel, salinity, turbidity, tds, waterTemp);
        
//         if(validReading){
//             if (WiFi.status() == WL_CONNECTED) {
//                 Serial.println("Wifi and data ok");
//                 uploadData(jsonPayload);
//             }

            
//             else{
//                 Serial.println("Wifi failed, but valid data");
//                 saveDataToCSV(jsonPayload);
//                 Serial.println(jsonPayload);
                
//             }
                
//         }

//         else{
//             Serial.println("invalid data... ignoring wifi");
//             Serial.println(jsonPayload);

//         }
        
        
//         vTaskDelay(pdMS_TO_TICKS(5000)); // 5 seconds delay for next sensor read
//     }
// }



// String prepareJsonPayload(float pH, float oxygenLevel, float salinity, float turbidity, float tds, float temperature) {
//     StaticJsonDocument<256> doc;
//     doc["pH"] = isnan(pH) ? JsonVariant() : String(pH,3);
//     doc["oxygenLevel"] = isnan(oxygenLevel) ? JsonVariant() : String(oxygenLevel,3);
//     doc["salinity"] = isnan(salinity) ? JsonVariant() : String(salinity,3);
//     doc["turbidity"] = isnan(turbidity) ? JsonVariant() : String(turbidity,3);
//     doc["TDS"] = isnan(tds) ? JsonVariant() : String(tds,3);
//     doc["temperature"] = isnan(temperature) ? JsonVariant() : String(temperature,3);

//     String jsonPayload;
//     serializeJson(doc, jsonPayload);
//     return jsonPayload;
// }


// void saveDataToCSV(String data) {
//     Serial.println("Saving data to CSV...");
//     File file = SPIFFS.open("/data.csv", FILE_APPEND);
//     if (!file) {
//         Serial.println("Failed to open file for writing");
//         return;
//     }
//     if(file.println(data)) {
//         Serial.println("Data saved successfully.");
//     } else {
//         Serial.println("Failed to save data.");
//     }
//     file.close();
// }


// String readDataFromCSV() {
//     File file = SPIFFS.open("/data.csv");
//     if (!file) {
//         Serial.println("There was an error opening the file for reading");
//         return "";
//     }
//     String data;
//     while (file.available()) {
//         data += file.readStringUntil('\n') + "\n";
//     }
//     file.close();
//     return data;
// }


// void uploadData(String jsonData) {
//     if (WiFi.status() == WL_CONNECTED) {
//         Serial.println("Sending to Google.");
//         HTTPClient http;
//         http.begin(WEB_APP_URL);
//         http.addHeader("Content-Type", "application/json");
//         int httpResponseCode = http.POST(jsonData);
//         // if (httpResponseCode > 0) {
//         //     Serial.print("HTTP Response code: ");
//         //     Serial.println(httpResponseCode);
//         // } else {
//         //     Serial.print("Error code: ");
//         //     Serial.println(httpResponseCode);
//         // }
//         http.end();
//     } else {
//         Serial.println("WiFi is not connected. Skipping data upload.");
//     }
// }





// void dataTransmissionTask(void *pvParameters) {
//     const TickType_t xFrequency = pdMS_TO_TICKS(30000); // Check every 30 seconds
//     TickType_t xLastWakeTime = xTaskGetTickCount();

//     while (1) {
//         if (WiFi.status() == WL_CONNECTED) {
//             // Serial.println("Sending csv.");
//             if (SPIFFS.exists("/data.csv")) {
//                 String data = readDataFromCSV();
//                 if (!data.isEmpty()) {
//                     // Serial.println("Data found in CSV. Sending stored data...");
//                     uploadData(data); // This function will handle sending the data
//                     SPIFFS.remove("/data.csv"); // Clear the stored data after successful transmission
//                     Serial.println("Data sent from CSV.");
//                 }
//             }
//         } else {
//             Serial.println("WiFi is not connected. Data will be stored locally.");
//         }
//         vTaskDelayUntil(&xLastWakeTime, xFrequency);
//     }
// }



// bool validateSensorReadings(float& humidity, float& temperature, float& turbidity, float& salinity, float& tds, float& ph, float& oxygenLevel) {
//     // Improved validation logic
//     bool isValid = true;
//     if (isnan(humidity) || humidity < 0 || humidity > 100) {  
//         isValid = false;
//         // Serial.println("Invalid or No humidity sensor detected. Setting default value.");
//         humidity = 0.0;  // Default value
//     }
//     if (isnan(temperature) || temperature < -40 || temperature > 85) {  
//         // Serial.println("Invalid or No temperature sensor detected. Setting default value.");
//         isValid = false;
//         temperature = 0.0;  // Default value
//     }
//     if (isnan(turbidity) || turbidity < 0) {  
//         // Serial.println("Invalid or No turbidity sensor detected. Setting default value.");
//         isValid = false;
//         turbidity = 0.0;  // Default value
//     }
//     if (isnan(salinity) || salinity < 0) {  
//         // Serial.println("Invalid or No salinity sensor detected. Setting default value.");
//         isValid = false;
//         salinity = 0.0;  // Default value
//     }
//     if (isnan(tds) || tds < 0) {  
//         // Serial.println("Invalid or No TDS sensor detected. Setting default value.");
//         isValid = false;
//         tds = 0.0;  // Default value
//     }
//      if (isnan(ph) || ph < 0) {  
//         // Serial.println("Invalid or No TDS sensor detected. Setting default value.");
//         isValid = false;
//         ph = 0.0;  // Default value
//     }
//     if (isnan(oxygenLevel) || oxygenLevel < 0) {  
//         // Serial.println("Invalid or No TDS sensor detected. Setting default value.");
//         isValid = false;
//         oxygenLevel = 0.0;  // Default value
//     }
//     return isValid;
// }


// void printStackSpace() {
//     UBaseType_t stackSpace = uxTaskGetStackHighWaterMark(NULL);
//     Serial.print("Remaining stack space: ");
//     Serial.println(stackSpace);
// }

// void blinkLED(int delayTime) {
//     // Serial.print("BLINKING\n");
//   // Turn LED on and off with a delay to create a blink
//   digitalWrite(STATUS_LED_PIN, HIGH);
//   delay(delayTime);
//   digitalWrite(STATUS_LED_PIN, LOW);
//   delay(delayTime);
// }

// void setLEDSolid(bool on) {
//   // Set the LED to solid on or off
//   digitalWrite(STATUS_LED_PIN, on ? HIGH : LOW);
// }

// void printDataFromCSV() {
//     if (SPIFFS.exists("/data.csv")) {
//         Serial.println("Printing data from CSV:");
//         File file = SPIFFS.open("/data.csv");
//         if (!file) {
//             Serial.println("Error opening file for reading.");
//             return;
//         }
//         while (file.available()) {
//             String line = file.readStringUntil('\n');
//             Serial.println(line);
//         }
//         file.close();
//     } else {
//         Serial.println("No data.csv file found.");
//     }
// }


// // void loop() {
    

// //     int buttonState = digitalRead(BUTTON_PIN);
// //     if(buttonState = LOW){
// //         tbdty.calibrate();

// //         //simple debounce
// //         while (digitalRead(BUTTON_PIN) == LOW) {
// //             delay(50);
// //         }
    
// //     }

// //     if (WiFi.status() == WL_CONNECTED) {
// //         // WiFi connected, set LED solid
// //         setLEDSolid(true);
// //     } else {
// //         // WiFi not connected
// //         if (WiFi.status() == WL_IDLE_STATUS) {
// //             // Initial connection attempt, blink twice
// //             blinkLED(200); // Short blink
// //             delay(200);
// //             blinkLED(200); // Short blink
// //             delay(200);
// //         } else {
// //             // WiFi connection failed, continue blinking
// //             blinkLED(500); // Blink every 500 ms
// //         }
// //     }

// //     delay(100); // Add a small delay to prevent excessive CPU usage
// // }