#include <Arduino.h>
#include "io_handler.h"
#include "rtc_handler.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include "TempSensor.h"
#include "dallasTemperature.h"
#include <Adafruit_MCP23X17.h>
#include "globals.h"
#include "I2Cadc.h"

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

    #ifdef I2C_DEBUG
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
    #endif

    Serial.println("TEMP&HUM");
    data.temperatureValid = temp.readTemperature(FAHRENHEIT, &data.temperature);
    data.humidityValid = temp.readHumidity(&data.humidity);

    Serial.println("Turb");
    data.turbidityValid = tbdty.readTurbidity(&data.turbidity);

    Serial.println("PH");
    data.pHValid = phGloabl.readpH(&data.pH);

    Serial.println("SAL");
    data.ecValid = sal.readEC(&data.ec);
    data.tdsValid = sal.readTDS(&data.tds);
    data.salinityValid = sal.readSalinity(&data.salinity);

    //Temp and salinity must be run in order to get proper reading for DO
    Serial.println("DO");
    data.oxygenLevelValid = DO.readDO(&data.oxygenLevel, data.salinity, data.temperature);

    // Round readings
    data.temperature = round(data.temperature * 1000.0) / 1000.0;
    data.turbidity = round(data.turbidity * 1000.0) / 1000.0;  
    data.pH = round(data.pH * 1000.0) / 1000.0;
    data.oxygenLevel = round(data.oxygenLevel * 1000.0) / 1000.0;   
    data.tds = round(data.tds * 1000.0) / 1000.0;
    data.ec = round(data.ec*1000.0) / 1000.0;
    data.salinity = round(data.salinity * 1000.0) / 1000.0;     
    data.humidity = round(data.humidity * 1000.0) / 1000.0;

    Serial.println("Sensor readings complete.");
}


bool uploadData(String jsonData) {
    int responseCode = 200;
    // if(xSemaphoreTake(simCardMutex, pdMS_TO_TICKS(5000)))    {
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
            if(!sim.isConnected())  {   //checks if sim connected to network, and tries to connnect if not
                Serial.println("Not connected to cellular network. Data not uploaded.");
                if(sim.isGprsConnected())   {   //if still not connected, will skip 
                    Serial.println("Will attempt to upload in next loop.");
                }else   {
                    Serial.println("Still not connected to gprs, will skip and try to connect next cycle.");
                    return false;
                }
                // xSemaphoreGive(simCardMutex);
            }else   {
                if(sim.connected == false)  {   //if not connected to domain, attmpt to connect to domain
                    if(!sim.serverConnect(server, resource))    {
                        Serial.println("Server could not connect");
                        sim.connected=false;
                        // xSemaphoreGive(simCardMutex);
                        return false;
                    }
                    else{ 
                        sim.connected = true;
                        sim.setJsonHeader();        //set headers
                        Serial.println("Server Connected!");
                    }
               // }else   {
                    Serial.println("Is server connected?");
                    if(!sim.IsServerConnected())    {   //verifies if domain connected
                        Serial.println("Server is not connected... Attmpting to connect on next cycle.");
                        // xSemaphoreGive(simCardMutex);
                        return false;
                    }else   {
                        Serial.println("attempting to send data");
                        sim.sendPostRequest(jsonData);      //attmpts to send json
                        Serial.println("Sent data successfully!");
                    }
                }
            }
        #endif

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

void validateSensorReadings(SensorData& data) {     //validates sensor readings
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
    const tm& timeinfo = get_current_time();    //gets current time
    StaticJsonDocument<1024> doc;

    //Creates readInfo array
    JsonArray jsArr = doc.createNestedArray("readInfo");        //creates huge array called readInfo

    //creates humidity sensor object
    JsonObject humDoc = jsArr.createNestedObject();     //stores object in array for humidity
    humDoc["id"] = "";
    humDoc["type"] = "float";
    humDoc["value"] = String(data.humidity,3);

    //creates a temperature sensor object
    JsonObject tempDoc = jsArr.createNestedObject();    //object for temp
    tempDoc["id"] = "pending";
    tempDoc["type"] = "float";
    tempDoc["value"] = String(data.temperature,3);

    //creates a turbidity sensor object
    JsonObject turbDoc = jsArr.createNestedObject();   //object for turbidity
    turbDoc["id"] = "pending";
    turbDoc["type"] = "float";
    turbDoc["value"] = String(data.turbidity, 3);

    //creates a salinity sensor object
    JsonObject salDoc = jsArr.createNestedObject();     //object for salinity
    salDoc["id"] = "pending";
    salDoc["type"] = "float";
    salDoc["value"] = String(data.salinity,3);

    //creates a conductivity sensor object
    JsonObject ecDoc = jsArr.createNestedObject();      //object for EC
    ecDoc["id"] = "pending";
    ecDoc["type"] = "float";
    ecDoc["value"] = String(data.ec,3);

    //creates a tds object
    JsonObject tdsDoc = jsArr.createNestedObject();     //object for TDS
    tdsDoc["id"] = "pending";
    tdsDoc["type"] = "float";
    tdsDoc["value"] = String(data.tds,3);

    //creates a ph object
    JsonObject phDoc = jsArr.createNestedObject();      //object for pH
    phDoc["id"] = "pending";
    phDoc["type"] = "float";
    phDoc["value"] = String(data.pH, 3);

    //creates a dissolved oxygen object
    JsonObject doDoc = jsArr.createNestedObject();      //object for DO
    doDoc["id"] = "pending";
    doDoc["type"] = "float";
    doDoc["value"] = String(data.oxygenLevel, 3);

    // Add date object
    JsonObject dateObj = doc.createNestedObject("date");    //object for date
    dateObj["year"] = String(timeinfo.tm_year);
    dateObj["month"] = String(timeinfo.tm_mon + 1);
    dateObj["day"] = String(timeinfo.tm_mday);
    dateObj["hour"] = String(timeinfo.tm_hour);
    dateObj["minute"] = String(timeinfo.tm_min);
    dateObj["second"] = String(timeinfo.tm_sec);

    // Add arrayInfo object
    JsonObject arrayInfo = doc.createNestedObject("arrayInfo");     //object for sensor array ID
    arrayInfo["id"] = "1";
    arrayInfo["passphrase"] = "randomText";

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    return jsonPayload;

    /* FOR CELLULAR AT COMMANDS*/
    // doc[KEY_HUMIDITY] = String(data.humidity, 3);
    // doc[KEY_TEMPERATURE] = String(data.temperature, 3);
    // doc[KEY_TURBIDITY] = String(data.turbidity, 3);
    // doc[KEY_SALINITY] = String(data.salinity, 3);
    // doc[KEY_TDS] = String(data.tds, 3);
    // doc[KEY_PH] = String(data.pH, 3);
    // doc[KEY_OXYGEN_LEVEL] = String(data.oxygenLevel, 3);
    // doc[KEY_MONTH] = String(timeinfo.tm_mon+1);
    // doc[KEY_DAY] = String(timeinfo.tm_mday);
    // doc[KEY_YEAR] = String(timeinfo.tm_year);
    // doc[KEY_HOUR] = String(timeinfo.tm_hour);
    // doc[KEY_MINUTE] = String(timeinfo.tm_min);
    // doc[KEY_SECOND] = String(timeinfo.tm_sec);
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
        if(!fs.exists(directoryPath))  {                    //Checks for directory in SD card
            fs.mkdir(directoryPath);
            Serial.println("made directory for csv!");
        }

        String filename;
        if (!is_time_synced()) {        //Checks to see if the time is synced to NTP
            Serial.println("Failed to get local time.");
            filename = directoryPath + "/unknown-time.csv";     //saves to generic directory
            getCurrentTime(timeinfo);
            // return false;
        }else   {
            updateSystemTime(timeinfo);                     //Updates current time
            timeinfo = get_current_time();                  //sets time to variable
            filename = directoryPath + "/" + String(timeinfo.tm_mon+1) + '-' + String(timeinfo.tm_mday) + '-' + String(timeinfo.tm_year) + "-data.csv"; //directory path
        }

        file = fs.open(filename, FILE_APPEND);  //opens the directory path to append
        if(!file)    {       //if cant open file to append/doesn't exist, create said file and write the headers
            Serial.println("Couldnt open file to append/write. Creating new file");
            String header = "Humidity, Temperature, Turbidity, Salinity, TDS, pH, Disolved Oxygen, Month, Day, Year, Time"; 
            file = fs.open(filename, FILE_WRITE, true);
            file.println(header);
        }else{
            Serial.println("Opened file for appending!");
        }
           
        if(file.println(data)) {        //checks if it can print data to SD
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
        File file;
        Serial.println(data);

        String directoryPath = JSON_DIR_PATH;
        if(!fs.exists(directoryPath))  {        //checks for the directory
            fs.mkdir(directoryPath);            //makes directory  
            Serial.println("made directory for json!");
        }

        // root = fs.open(JSON_DIR_PATH);
        // if (!root) {
        //     fs.mkdir(directoryPath);
        // }
        
        String filename;
        timeinfo = get_current_time();      //sets current time to time variable to read from.
        if (is_time_synced() == false) {    //checks if time has been synced
            Serial.println("Failed to get local time.");
            Serial.println("Data will not be saving in JSON format.");
        }else   {
            //updateSystemTime(timeinfo);
            filename = String(directoryPath) + "/" + (timeinfo.tm_mon + 1) + '-' + timeinfo.tm_hour + '-' + (timeinfo.tm_year) + "-data.json";      //Sets file name
            if(!(file = fs.open(filename, FILE_APPEND))) {  //Checks if the file opens for appending in the directory
                Serial.println("Failed to open JSON file for writing.");
                file = fs.open(filename, FILE_WRITE, true);     //opens file for writing if cannot open for appending
            }

            if (file.println(data)) {       //Checks if it can write data to the file
                Serial.println("Data saved successfully.");
            } else {
                Serial.println("Failed to save data.");
                Serial.println(file.println());
            }

        }

        
        file.close();
        xSemaphoreGive(sdCardMutex);        //gives access for the Sd card and sim card mutex for use in other functions
        xSemaphoreGive(simCardMutex);
        return true;
    } else {
        Serial.println("Failed to obtain SD Card mutex for writing JSON.");
        return false;
    }
}

String readDataFromSD(fs::FS &fs, const char* fileName) {
    if (xSemaphoreTake(sdCardMutex, pdMS_TO_TICKS(5000))) {     //takes semaphore so no other function can use SD card
        File file = fs.open(fileName, FILE_READ);       //opens file
        if (!file) {
            Serial.println("Failed to open file for reading");
            xSemaphoreGive(sdCardMutex);
            return String();
        }

        String data = file.readStringUntil('\n');   //reads data
        file.close();
        xSemaphoreGive(sdCardMutex);
        return data;    //returns dataa
    } else {
        Serial.println("Failed to obtain SD Card mutex for reading.");
        return String();
    }
}