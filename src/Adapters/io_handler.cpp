#include <Arduino.h>
#include "io_handler.h"
#include "rtc_handler.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include "TempSensor.h"
#include "dallasTemperature.h"
#include <Adafruit_MCP23X17.h>

// extern bool isConnected;

extern SemaphoreHandle_t sdCardMutex;
extern SemaphoreHandle_t sensorMutex;
extern SemaphoreHandle_t simCardMutex;
const char* DATA_URL = "https://smart-seawall-server-4c5cb6fd8f61.herokuapp.com/api/data";
// const char* DATA_URL = "https://smart-seawall-server-4c5cb6fd8f61.herokuapp.com/api/test-data";

const char server[]   = "https://d17e66a7-c349-4d03-9453-cf90701e7aaa.mock.pstmn.io";
const char resource[] = "/post";
const int  port       = 443;

// Define these in a suitable header file or at the top of your source file
const char* KEY_HUMIDITY = "humidity";
const char* KEY_TEMPERATURE = "temperature";
const char* KEY_TURBIDITY = "turbidity";
const char* KEY_SALINITY = "salinity";
const char* KEY_TDS = "tds";
const char* KEY_PH = "pH";
const char* KEY_OXYGEN_LEVEL = "oxygenLevel";
const char* KEY_MONTH = "Month";
const char* KEY_DAY = "Day";
const char* KEY_YEAR = "Year";
const char* KEY_HOUR = "Hour";
const char* KEY_MINUTE = "Minute";
const char* KEY_SECOND = "Second";

extern const char* JSON_DIR_PATH ;
extern const char* CSV_DIR_PATH ;

void readSensorData(SensorData &data)
{

    Serial.println("Reading sensor data...");
    // pinMode(40,OUTPUT);
    // digitalWrite(40,HIGH);
    byte error, address;
    int nDevices;
    Serial.println("Scanning...");
    nDevices = 0;
    for(address = 0; address < 127; address++ ) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0) {
        Serial.print("I2C device found at address 0x");
        if (address<16) {
            Serial.print("0");
        }
        Serial.println(address,HEX);
        nDevices++;
        }
        else if (error==4) {
        Serial.print("Unknow error at address 0x");
        if (address<16) {
            Serial.print("0");
        }
        Serial.println(address,HEX);
        }    
    }
    if (nDevices == 0) {
        Serial.println("No I2C devices found\n");
    }
    else {
        Serial.println("done\n");
    }

    delay(5000);
    // Serial.println("Turb");
    // data.turbidityValid = tbdty.readTurbidity(&data.turbidity);
    // Serial.println("PH");
    // data.pHValid = phGloabl.readpH(&data.pH);
    // Serial.println("DO");
    // data.oxygenLevelValid = DO.readDO(&data.oxygenLevel, data.salinity, data.temperature);
    // Serial.println("SAL");
    // data.ecValid = sal.readEC(&data.ec);
    // data.tdsValid = sal.readTDS(&data.tds);
    // data.salinityValid = sal.readSalinity(&data.salinity);

    Serial.println("TEMP&HUM");
    data.temperatureValid = temp.readTemperature(FAHRENHEIT, &data.temperature);
    data.humidityValid = temp.readHumidity(&data.humidity);

  

    // Round readings
    data.temperature = round(data.temperature * 1000.0) / 1000.0;
    data.turbidity = round(data.turbidity * 1000.0) / 1000.0;   //MUST BE BEFORE PH
    data.pH = round(data.pH * 1000.0) / 1000.0;
    data.oxygenLevel = round(data.oxygenLevel * 1000.0) / 1000.0;   //MUST BE BEFORE EC
    data.tds = round(data.tds * 1000.0) / 1000.0;
    data.ec = round(data.ec*1000.0) / 1000.0;
    data.salinity = round(data.salinity * 1000.0) / 1000.0;     //MUST BE LAST
    data.humidity = round(data.humidity * 1000.0) / 1000.0;

    Serial.println("Sensor readings complete.");
}


bool uploadData(String jsonData) {
    int responseCode = 200;
    if(xSemaphoreTake(simCardMutex, pdMS_TO_TICKS(5000)))    {
        #ifndef CELLULAR
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Not connected to WiFi. Data not uploaded.");
            return false;
        }
        Serial.println("Uploading: "+jsonData); 
        
        HTTPClient http;
        http.begin(DATA_URL);
        http.addHeader("Content-Type", "application/json");
        ResponseCode = http.POST(jsonData);
        http.end();
        #else
            if(!sim.isConnected())  {
                Serial.println("Not connected to cellular network. Data not uploaded.");
                xSemaphoreGive(simCardMutex);
                return false;
            }else   {
                if(sim.connected == false)  {
                    if(!sim.serverConnect(server, resource))    {
                        Serial.println("Server could not connect");
                        sim.connected=false;
                        xSemaphoreGive(simCardMutex);
                        return false;
                    }
                    else{
                        sim.connected = true;
                        sim.setJsonHeader();
                        Serial.println("Server Connected!");
                    }
                }else   {
                    Serial.println("Is server connected?");
                    if(!sim.IsServerConnected())    {
                        Serial.println("Server is not connected... Attmpting to connect on next cycle.");
                        xSemaphoreGive(simCardMutex);
                        return false;
                    }else   {
                        Serial.println("attempting to send data");
                        sim.sendPostRequest(jsonData);
                        Serial.println("Sent data successfully!");
                    }
                }
            }
        #endif
        xSemaphoreGive(simCardMutex);
    }
    
    // if (httpResponseCode > 0) {
    //     String response = http.getString();
    //     Serial.println("HTTP Response code: " + String(httpResponseCode));
    //     Serial.println(response);
    // } else {
    //     Serial.println("HTTP POST request failed.");
    // }
    return (responseCode == 200 || responseCode == 201);
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
    toPrint+= "|Turbidity: "+String(data.turbidity,3)+"\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint+= "|TDS: "+String(data.tds,3)+"\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint+= "|EC: "+String(data.ec,3)+"\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint+= "|Oxigen Level: "+String(data.oxygenLevel,3)+"\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint+= "|PH: "+String(data.pH,3)+"\n";
    toPrint += "+-----------------------+-----------------------+\n";
    toPrint+= "|Humidity: "+String(data.humidity,3)+"\n";
    toPrint += "+-----------------------+-----------------------+\n";

    Serial.println(toPrint);
}

void validateSensorReadings(SensorData& data) {
    data.humidityValid = !isnan(data.humidity) && data.humidity >= 0 && data.humidity <= 100;
    data.temperatureValid = !isnan(data.temperature) && data.temperature >= -40 && data.temperature <= 85;
    data.turbidityValid = !isnan(data.turbidity) && data.turbidity >= 0;
    data.salinityValid = !isnan(data.salinity) && data.salinity >= 0;
    data.tdsValid = !isnan(data.tds) && data.tds >= 0;
    data.ecValid = !isnan(data.ec) && data.ec >= 0;
    data.pHValid = !isnan(data.pH) && data.pH >= 0;
    data.oxygenLevelValid = !isnan(data.oxygenLevel) && data.oxygenLevel >= 0;
}



String prepareJsonPayload(const SensorData& data) {
    const tm& timeinfo = get_current_time();
    StaticJsonDocument<256> doc;
    doc[KEY_HUMIDITY] = String(data.humidity, 3);
    doc[KEY_TEMPERATURE] = String(data.temperature, 3);
    doc[KEY_TURBIDITY] = String(data.turbidity, 3);
    doc[KEY_SALINITY] = String(data.salinity, 3);
    doc[KEY_TDS] = String(data.tds, 3);
    doc[KEY_PH] = String(data.pH, 3);
    doc[KEY_OXYGEN_LEVEL] = String(data.oxygenLevel, 3);
    doc[KEY_MONTH] = String(timeinfo.tm_mon+1);
    doc[KEY_DAY] = String(timeinfo.tm_mday);
    doc[KEY_YEAR] = String(timeinfo.tm_year);
    doc[KEY_HOUR] = String(timeinfo.tm_hour);
    doc[KEY_MINUTE] = String(timeinfo.tm_min);
    doc[KEY_SECOND] = String(timeinfo.tm_sec);
    String jsonPayload;
    serializeJson(doc, jsonPayload);

    /* FOR CELLULAR AT COMMANDS*/
    jsonPayload.replace("\"", "\\\"");
    jsonPayload = "\"" + jsonPayload + "\""; 
    return jsonPayload;
}

String prepareCSVPayload(const SensorData& data)    {
    const tm& timeinfo = get_current_time();
    return String(data.humidity, 3) + ", " + String(data.temperature, 3) +
        ", " + String(data.turbidity, 3) + ", " + String(data.salinity, 3) + 
        ", " + String(data.tds, 3) + ", " + String(data.ec, 3) + ", "
        ", " + String(data.pH, 3) + ", " +
        String(data.oxygenLevel, 3) + ", " + 
        (timeinfo.tm_mon+1) + ", " +
        timeinfo.tm_mday + ", " + 
        (timeinfo.tm_year) + ", " + 
        timeinfo.tm_hour + ":" + timeinfo.tm_min + ":" + timeinfo.tm_sec;   
}

bool saveCSVData(fs::FS &fs, const String& data) {
    if (xSemaphoreTake(sdCardMutex, pdMS_TO_TICKS(5000)) && xSemaphoreTake(simCardMutex, pdMS_TO_TICKS(5000))) {
        tm timeinfo;

        Serial.println("Saving data to CSV file...");

        
        String directoryPath = CSV_DIR_PATH;
        File file;
        if(!fs.exists(directoryPath))  {
            fs.mkdir(directoryPath);
            Serial.println("made directory for csv!");
        }

        String filename;
        if (!is_time_synced()) {
            Serial.println("Failed to get local time.");
            filename = directoryPath + "/unknown-time.csv";
            getCurrentTime(timeinfo);
            // return false;
        }else   {
            updateSystemTime(timeinfo);
            timeinfo = get_current_time();
            filename = directoryPath + "/" + String(timeinfo.tm_mon+1) + '-' + String(timeinfo.tm_mday) + '-' + String(timeinfo.tm_year) + "-data.csv";
        }

        file = fs.open(filename, FILE_APPEND);
        if(!file)    {       //if cant open file to append/doesn't exist, create said file and write the headers
            Serial.println("Couldnt open file to append/write. Creating new file");
            String header = "Humidity, Temperature, Turbidity, Salinity, TDS, pH, Disolved Oxygen, Month, Day, Year, Time";
            file = fs.open(filename, FILE_WRITE, true);
            file.println(header);
        }else{
            Serial.println("Opened file for appending!");
        }
           
        if(file.println(data)) {
            Serial.println("Data saved successfully.");
        } 
        else {
            Serial.println("Failed to save data.");
        }

        file.close();
        xSemaphoreGive(sdCardMutex);
        xSemaphoreGive(simCardMutex);
        return true;
    }else {
        Serial.println("Failed to obtain SD Card mutex for writing.");
        return false;
    }
}

bool saveJsonData(fs::FS &fs, const String &data) {
    if (xSemaphoreTake(sdCardMutex, pdMS_TO_TICKS(5000)) && xSemaphoreTake(simCardMutex, pdMS_TO_TICKS(5000))) {
        struct tm timeinfo;
        Serial.println("Saving data to JSON file...");
        Serial.println(data);
        File file;

        String directoryPath = JSON_DIR_PATH;
        if(!fs.exists(directoryPath))  {
            fs.mkdir(directoryPath);
            Serial.println("made directory for json!");
        }

        // root = fs.open(JSON_DIR_PATH);
        // if (!root) {
        //     fs.mkdir(directoryPath);
        // }
        
        String filename;
        if (!is_time_synced) {
            Serial.println("Failed to get local time.");
            Serial.println("Data will not be saving in JSON format.");
        }else   {
            //updateSystemTime(timeinfo);
            timeinfo = get_current_time();
            filename = String(directoryPath) + "/" + (timeinfo.tm_mon + 1) + '-' + timeinfo.tm_hour + '-' + (timeinfo.tm_year) + "-data.json";
            if(!(file = fs.open(filename, FILE_APPEND))) {
                Serial.println("Failed to open JSON file for writing.");
                file = fs.open(filename, FILE_WRITE, true);
            }

            if (file.println(data)) {
                Serial.println("Data saved successfully.");
            } else {
                Serial.println("Failed to save data.");
                Serial.println(file.println());
            }
        }

        
        file.close();
        xSemaphoreGive(sdCardMutex);
        xSemaphoreGive(simCardMutex);
        return true;
    } else {
        Serial.println("Failed to obtain SD Card mutex for writing JSON.");
        return false;
    }
}

String readDataFromSD(fs::FS &fs, const char* fileName) {
    if (xSemaphoreTake(sdCardMutex, pdMS_TO_TICKS(5000))) {
        File file = fs.open(fileName, FILE_READ);
        if (!file) {
            Serial.println("Failed to open file for reading");
            xSemaphoreGive(sdCardMutex);
            return String();
        }

        String data = file.readStringUntil('\n');
        file.close();
        xSemaphoreGive(sdCardMutex);
        xSemaphoreGive(simCardMutex);
        return data;
    } else {
        Serial.println("Failed to obtain SD Card mutex for reading.");
        return String();
    }
}


// String readDataFromSD(fs::FS &fs, const char* fileName) {
//     if (xSemaphoreTake(sdCardMutex, pdMS_TO_TICKS(5000))) {
//         File file = fs.open(fileName, FILE_READ);
//         if (!file) {
//             Serial.println("Failed to open file for reading");
//             xSemaphoreGive(sdCardMutex);
//             return String();
//         }

//         String data = file.readStringUntil('\n');
//         file.close();
//         xSemaphoreGive(sdCardMutex);
//         return data;
//     } else {
//         Serial.println("Failed to obtain SD Card mutex for reading.");
//         return String();
//     }
// }

