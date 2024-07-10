// #include <Arduino.h>
// #include <Wire.h>
// #include "SPIFFS.h"
// #include <ArduinoJson.h>
// #include "SPI.h"
// #include <HardwareSerial.h>
// #include "FS.h"
// #include "SdFat.h"
// //#include <ESP32SPISlave.h>
// #include "esp_sntp.h"
// // #include "NTPClient.h"
// #include "WiFiUdp.h"
// #include <WiFi.h>
// #include <HTTPClient.h>


// // Sensor headers
// // #include "TempSensor.h"
// // #include "TurbiditySensor.h"
// // #include "SalinitySensor.h"
// // #include "pHSensor.h"
// // #include <base_surveyor.h>
// // #include "DOSensor.h"

// #define SLEEP_TIME_US 30000000 // 1 minute = 60000000 - multiply by any number for amount of minutes
// #define I2C_ADR 0x10

// void sendToEsp32Slave(SPIClass *spi, byte data[], int length);

// SdFat32 SD;

// const char* SSID = "Jesus Loves You";
// const char* PASSWD = "3052354491";
// const char* WEB_APP_URL = "https://script.google.com/macros/s/AKfycbzdREDYLRb1ew8CjwGY_WnrIU0UWW0Sn3Wr4XdT8Jv0VjXuQxJV7GVCKZeYtEb2zrKb/exec";
// const char *serverName = "https://smart-seawall-server-4c5cb6fd8f61.herokuapp.com/api/data";
// //const char *serverName = "MONGODB_URI=mongodb+srv://lisettehawkins09:cxO0hBBXellzkuAX@cluster0-sensordatassam.sk9l59s.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0-sensorDatasSample";

// const int enable = 26;
// const int STATUS_LED_PIN = 2; // Commonly the onboard LED pin on ESP32
// const int BUTTON_PIN = 0;
// byte mydata = 0;
// byte sendTime[4];

// // Struct for sensor data
// struct SensorData {
//     float humidity;
//     bool humidityValid;
//     float temperature;
//     bool temperatureValid;
//     float turbidity;
//     bool turbidityValid;
//     float salinity;
//     bool salinityValid;
//     float tds;
//     bool tdsValid;
//     float pH;
//     bool pHValid;
//     float oxygenLevel;
//     bool oxygenLevelValid;
// };   

// // NTP clock updating stuff maybe???? idk if it actually works im just trying stuff at this point AJAJJAJAJAJAJ
// const char* ntpServer1 = "pool.ntp.org";
// const char* ntpServer2 = "time.nist.gov";
// const long gmtOffset_sec = -(3600 * 5);
// const int daylightOffset_sec = 3600;
// const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";
// struct tm timeinfo;
// const int yearOffset = 1900;


// // Forward declarations
// void saveDataToJSONFile(String data);
// void uploadData(String data);
// String prepareJsonPayload(const SensorData& data);
// String prepareCSVPayload(const SensorData& data);
// void saveCSVData(String data);
// String readDataFromJSONFile();
// void validateSensorReadings(SensorData& data);
// void printDataOnCLI(const SensorData& data);
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

// void printLocalTime()   {
//     if(!getLocalTime(&timeinfo))    {
//         Serial.println("No time available (yet)");
//         return;
//     }
//     Serial.println(&timeinfo, "%A, %B %d, %Y %H:%M:%S");
// }

// // Callback function (get's called when time adjusts via NTP)
// void timeavailable(struct timeval *t)
// {
//   Serial.println("Got time adjustment from NTP!");
//   printLocalTime();
// }

// void sendDataI2C()  {
//     Wire.beginTransmission(0x10);
//     Wire.write((const uint8_t *)&timeinfo, sizeof(tm));
//     Wire.endTransmission();
//     Serial.println("Time sent to slave");
// }

// int light = 0;

// void setup() {
//     //setCpuFrequencyMhz(80);
//     Serial.begin(115200);
//     //Serial3.begin(115200, SERIAL_8N1, 16, 17);  //pins 16 rx2, 17 tx2, 8 bits, 1 parity bit
//     //Wire.begin();
//     // Initialize SPIFFS
//     if (!SPIFFS.begin(true)) {
//         Serial.println("An Error has occurred while mounting SPIFFS");
//         return;
//     } else {
//         Serial.println("SPIFFS mounted successfully.");
//     }

//     Wire.begin();
//     WiFi.begin(SSID, PASSWD);

//     sntp_set_time_sync_notification_cb(timeavailable);
//     configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

// }

// void loop() {
//     delay(1000);
//     if(WiFi.status == 0)    {
//         blinkLED(200);
//     }
//     sendDataI2C();
//     getLocalTime(&timeinfo);

//     // if(mydata == 0) {
//     //     mydata = 1;
//     // }else   {
//     //     mydata = 0;
//     // }

//     // Wire.beginTransmission(0x10);
//     // Wire.write(mydata);
//     // Wire.endTransmission();
//     // delay(1000);
    

//     // delay(2500);
//     // digitalWrite(enable, LOW);
//     // Serial.println("OFF");
//     // delay(2500);
//     // digitalWrite(enable, HIGH);
//     // Serial.println("ON");

    

//     // if(cardMount != true)   {
//     //     if(!SD.begin(SdSpiConfig(5, SHARED_SPI, SD_SCK_MHZ(16)))){
//     //         Serial.println("Card Mount Failed");
//     //     }else   {
//     //         Serial.println("Card mount sucessful!");
//     //         cardMount = true;
//     //     }   
//     // }
   


//     // Sensor data
//     // SensorData data = {0};   

//     // String jsonPayload = prepareJsonPayload(data);
//     // String csvPayLoad = prepareCSVPayload(data);
//     // printDataOnCLI(data);
//     // saveDataToJSONFile(jsonPayload);
//     // saveCSVData(csvPayLoad);
    


//     //delay(5000);
//     //Sleep Mode settings
//     //esp_sleep_config_gpio_isolate();
//     //esp_sleep_enable_timer_wakeup(SLEEP_TIME_US);
//     // esp_light_sleep_start();
//     //esp_deep_sleep_start();
// }

// void sendToEsp32Slave(SPIClass *spi, byte data[], int length)   {
//     spi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
//     digitalWrite(spi->pinSS(), LOW);

//     spi->transfer(data,length);
//     digitalWrite(spi->pinSS(), HIGH);
//     spi->endTransaction();
// }


// void printDataOnCLI(const SensorData& data){
//     // String toPrint="";
    
//     // // Header
//     // toPrint += "+-----------------------+-----------------------+\n";
//     // toPrint += "|Sensor Readings:\n";
//     // toPrint += "+-----------------------+-----------------------+\n";
//     // toPrint += "|Temperature: "+String(data.temperature,3)+"\n";
//     // toPrint += "+-----------------------+-----------------------+\n";
//     // toPrint += "|Salinity:"+String(data.salinity,3)+"\n";
//     // toPrint += "+-----------------------+-----------------------+\n";
//     // toPrint += "|Turbidity: "+String(data.turbidity,3)+"\n";
//     // toPrint += "+-----------------------+-----------------------+\n";
//     // toPrint += "pH Value: "+String(data.pH,3) + "\n";
//     // toPrint += "Humidity Value: "+String(data.humidity,3) + "\n";
//     // toPrint += "Oxygen Value: "+String(data.oxygenLevel,3) + "\n";

//     // Serial.println(toPrint);
// }

// String prepareJsonPayload(const SensorData& data) {
//     // StaticJsonDocument<512> doc;
//     // doc["pH"] = String(data.pH, 3);
//     // doc["oxygenLevel"] = String(data.oxygenLevel, 3);
//     // doc["turbidity"] = String(data.turbidity, 3);
//     // doc["temperature"] = String(data.temperature, 3);
//     // doc["salinity"] = String(data.salinity, 3);
//     // doc["tds"] = String(data.tds, 3);
//     // doc["humidity"] = String(data.humidity, 3);
//     // // doc["Month"] = String(timeinfo.tm_mon+1);
//     // // doc["Day"] = String(timeinfo.tm_mday);
//     // // doc["Year"] = String(timeinfo.tm_year + yearOffset);
//     // // doc["Hour"] = String(timeinfo.tm_hour);
//     // // doc["Minute"] = String(timeinfo.tm_min);
//     // // doc["Second"] = String(timeinfo.tm_sec);
//     // String jsonPayload;
//     // serializeJson(doc, jsonPayload);
//     return "";//jsonPayload;
// }

// String prepareCSVPayload(const SensorData& data)    {
//     return String(data.humidity, 3) + ", " + String(data.temperature, 3) +
//         ", " + String(data.turbidity, 3) + ", " + String(data.salinity, 3) + 
//         ", " + String(data.tds, 3) + ", " + String(data.pH, 3) + ", " +
//         String(data.oxygenLevel, 3); 
//         /*
//         + ", " + (timeinfo.tm_mon+1) + ", " +
//         timeinfo.tm_mday + ", " + (timeinfo.tm_year + yearOffset) + ", " + timeinfo.tm_hour +
//         ":" + timeinfo.tm_min + ":" + timeinfo.tm_sec;   
//         */
// }

// void saveCSVData(String data)   {
//     Serial.println("Saving data to CSV file...");
//     File32 file;

//     String filename;
//     // if(!getLocalTime(&timeinfo))    {
//     //     filename = "/csvFiles/unknown-time.csv"; 
//     // }else   {
//     //     filename = String("/csvFiles/") + (timeinfo.tm_mon+1) + '-' + timeinfo.tm_mday + '-' + (timeinfo.tm_year + yearOffset) + String("-data.csv"); 
//     // }
//     // if(!SD.open(filename, O_WRITE | O_APPEND))    {       //if cant open file to append/doesn't exist, create said file and write the headers
//     //     file = SD.open(filename, O_WRITE | O_CREAT);
//     //     String header = "Humidity, Temperature, Turbidity, Salinity, TDS, pH, Disolved Oxygen, Month, Day, Year, Hour";
//     //     file.println(header);
//     // }else{
//     //     file = SD.open(filename, O_WRITE | O_APPEND);
//     // }
//     // if (file.println(data)) {
//     //     Serial.println("Data saved successfully.");
//     // } else {
//     //     Serial.println("Failed to save data.");
//     // }
//     file.close();
// }

// void saveDataToJSONFile(String data) {
//     Serial.println("Saving data to JSON file...");
//     Serial.println(data);
//     File32 file;
//     String filename;
//     // if(!getLocalTime(&timeinfo))    {
//     //     filename = "/jsonFiles/unknown-time.json"; 
//     // }else   {
//     //     filename = String("/jsonFiles/") + (timeinfo.tm_mon+1) + '-' + timeinfo.tm_mday + '-' + (timeinfo.tm_year + yearOffset) + String("-data.json"); 
//     // }
//     // file = SD.open(filename, O_WRITE | O_CREAT | O_APPEND);
//     // if (file.println(data)) {
//     //     Serial.println("Data saved successfully.");
//     // } else {
//     //     Serial.println("Failed to save data.");
//     // }
//     file.close();
// }

// String readDataFromJSONFile() {
//     return "";
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

