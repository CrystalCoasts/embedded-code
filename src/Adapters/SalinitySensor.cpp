#include "SalinitySensor.h"
#include <iostream>

String EC_TAG = "[EC_SENSOR] ";
SalinitySensor& sal = SalinitySensor::Get();

//EC Object prototype
SalinitySensor::SalinitySensor() {}

void SalinitySensor::begin() {
    bool calibrated, tempComp, kFactor;
    char parsedData[32];

    //Turns on the device using ioExtender
    delay(500);
    mcpGlobal.pinModeA(EN_S,0);
    mcpGlobal.digitalWriteA(EN_S, HIGH);
    
    // Checks if calibrated
    ec.send_cmd("Cal,?");
    delay(500);
    ec.receive_cmd(ec_data, sizeof(ec_data));

    if (parseValue(ec_data, parsedData, "?Cal")) {
        String msg = EC_TAG + String ("Calibration profile detected.");
        Serial.println(msg);
        calibrated = (atoi(parsedData) != 0);
    }

    //Checks temperature compensation for accurate readings
    ec.send_cmd("T,?");
    delay(500);
    ec.receive_cmd(ec_data, sizeof(ec_data));

    if (parseValue(ec_data, parsedData, "?T")) {
        String msg = EC_TAG + String ("Correct Temp compensation.");
        Serial.println(msg);
        tempComp = (atoi(parsedData) == TEMP_COMP);
    }
    if (!tempComp) {
        String command = "T," + String(TEMP_COMP);
        ec.send_cmd(command.c_str());
        delay(500);
    }

    //Checks conversation factor for accurate readings
    ec.send_cmd("K,?");
    delay(500);
    ec.receive_cmd(ec_data, sizeof(ec_data));

    if (parseValue(ec_data, parsedData, "?K")) {
        String msg = EC_TAG + String ("Correct Conversion Factor.");
        Serial.println(msg);
        kFactor = (atoi(parsedData) == K_FACTOR);
    }
    if (!kFactor) {
        String command = "K," + String(K_FACTOR);
        ec.send_cmd(command.c_str());
        delay(500);
    }

    //Disables all readings that the sensor can do
    DisableAllReadings();
}

void SalinitySensor::EnableDisableSingleReading(uint8_t readOption, uint8_t data) {
    String command = "O,";
    String msg;
    delay(600);
    switch (readOption) {   //Turns on/off parameters to be read 
    case SAL:
        command += "S," + String(data);
        msg = EC_TAG + String ("Salinity reading status changed");
        Serial.println(msg);
        break;
    case EC:
        command += "EC," + String(data);
        msg = EC_TAG + String ("EC status changed");
        Serial.println(msg);
        break;
    case TDS:
        command += "TDS," + String(data);
        msg = EC_TAG + String ("TDS status changed");
        Serial.println(msg);
        break;
    case SG:
        command += "SG," + String(data);
        break;
    default:
        break;
    }

    ec.send_cmd(command.c_str());
    delay(500);
}

bool SalinitySensor::readSalinity(float* salinity) {
    wake(); //wakes device with ioExtender

    //enable only salinity reading
    delay(500);
    EnableDisableSingleReading(EC, 0);
    EnableDisableSingleReading(TDS, 0);
    EnableDisableSingleReading(SG, 0);
    EnableDisableSingleReading(SAL, 0);
    EnableDisableSingleReading(SAL, 1);

    //Sends read command
    delay(1200);
    ec.send_cmd("R");
    delay(600);
    ec.receive_cmd(ec_data, sizeof(ec_data)); //receive readings and stores in ec_data
    delay(500);
    sleep();    //sleeps with ioExtender
    if (ec_data[0] != '\0') {   //if not null
        *salinity = atof(ec_data);  //set pointer value to ec_data
        return true;    //sucessful reading
    }
    return false;   //unsucessful reading
}

bool SalinitySensor::readTDS(float* salinity){
    wake();     //wakes with ioExtender
    delay(500);
    
    //enable only TDS reading
    EnableDisableSingleReading(EC, 0);
    EnableDisableSingleReading(TDS, 0);
    EnableDisableSingleReading(SG, 0);
    EnableDisableSingleReading(SAL, 0);
    EnableDisableSingleReading(TDS, 1);

    delay(600);
    ec.send_cmd("R");   //sends read command
    delay(600);
    ec.receive_cmd(ec_data, sizeof(ec_data));   //receive sensor data
    sleep();    //sleeps with ioExtender
    if (ec_data[0] != '\0') {
        *salinity = atof(ec_data);  //sets pointer to the sensor data value
        return true;
    }
    return false;
}

bool SalinitySensor::readEC(float* salinity){
    wake();     //wakes using ioExtender
    delay(500);
    //enable only EC reading
    EnableDisableSingleReading(SG, 0);
    EnableDisableSingleReading(SAL, 0);
    EnableDisableSingleReading(TDS, 0);
    EnableDisableSingleReading(EC, 0);
    EnableDisableSingleReading(EC, 1);

    delay(600);
    ec.send_cmd("R");       //sends read command
    delay(600);
    ec.receive_cmd(ec_data, sizeof(ec_data));       //recieves sensor data
    sleep();        //sleps with ioExtender
    if (ec_data[0] != '\0') {
        *salinity = atof(ec_data);      //sets pointer to sensor data
        return true;
    }
    return false;
}

void SalinitySensor::calibrate()    {

    wake();     //wakes using ioExtender
    delay(300);

    //Clearing calibration parameters
    Serial.println("Clearing previous calibration...");
    ec.send_cmd("Cal,clear");
    delay(300);

    // 2 Step Calibration Code 
    Serial.println("Starting Salnity Dry Calibration");
    ec.send_cmd("Cal,dry");         //With no water
    delay(600);
    Serial.println("Finished, 10 seconds to switch to 12,880 low point calibration");
    delay(10000);
    ec.send_cmd("Cal,low,12880");       //Calibrate with 12,880 EC water
    delay(600);
    Serial.println("Finished low point, 10 seconds to switch to 80,000 high point calibration");
    delay(10000);

    ec.send_cmd("Cal,high,80000");      //Calibrate with 80,000 EC water
    delay(600);
    Serial.println("Salinity Calibration Complete!");

    sleep();


}

void SalinitySensor::DisableAllReadings() {
    ec.send_cmd("O,EC,0");
    delay(500);
    ec.send_cmd("O,TDS,0");
    delay(500);
    ec.send_cmd("O,S,0");
    salFlag = false;
    delay(500);
    ec.send_cmd("O,SG,0");
    delay(500);
    Serial.println("Cleared Readings.");
}



bool SalinitySensor::parseValue(const char* rawBuff, char* parsedBuff, const char* key) {
    const char* found = strstr(rawBuff, key);
    if (found) {
        strcpy(parsedBuff, found + strlen(key) + 1);
        return true;
    }
    return false;
}

SalinitySensor& SalinitySensor::Get() {
    static SalinitySensor instance;
    return instance;
}


void SalinitySensor::sleep() {
    mcpGlobal.digitalWriteA(EN_S, LOW);

}  

void SalinitySensor::wake() {
    mcpGlobal.digitalWriteA(EN_S, HIGH);
}