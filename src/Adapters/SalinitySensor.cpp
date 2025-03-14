#include "SalinitySensor.h"
#include <iostream>

String EC_TAG = "[EC_SENSOR] ";
SalinitySensor& sal = SalinitySensor::Get();

SalinitySensor::SalinitySensor() {}

void SalinitySensor::begin() {
    bool calibrated, tempComp, kFactor;
    char parsedData[32];
    delay(500);
    mcpGlobal.pinModeA(EN_S,0);
    mcpGlobal.digitalWriteA(EN_S, HIGH);
    
    ec.send_cmd("Cal,?");
    delay(500);
    ec.receive_cmd(ec_data, sizeof(ec_data));

    if (parseValue(ec_data, parsedData, "?Cal")) {
        String msg = EC_TAG + String ("Calibration profile detected.");
        Serial.println(msg);
        calibrated = (atoi(parsedData) != 0);
    }

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

    DisableAllReadings();
}

void SalinitySensor::EnableDisableSingleReading(uint8_t readOption, uint8_t data) {
    String command = "O,";
    String msg;
    delay(600);
    switch (readOption) {
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
    wake();
    //enable only salinity reading
    delay(500);
    EnableDisableSingleReading(EC, 0);
    EnableDisableSingleReading(TDS, 0);
    EnableDisableSingleReading(SG, 0);
    EnableDisableSingleReading(SAL, 0);
    EnableDisableSingleReading(SAL, 1);

    // ec.send_cmd("O,?");
    // delay(300);
    // ec.receive_cmd(ec_data, sizeof(ec_data));
    // char salStr[32];
    // if (!salFlag) {
    //     EnableDisableSingleReading(SAL, 1);

    // }
    delay(1200);
    ec.send_cmd("R");
    delay(600);
    ec.receive_cmd(ec_data, sizeof(ec_data));
    // Serial.print("RAW salinity Read: ");
    // Serial.println(ec_data);
    delay(500);
    sleep();
    if (ec_data[0] != '\0') {
        *salinity = atof(ec_data);
        return true;
    }
    return false;
}

bool SalinitySensor::readTDS(float* salinity){
    //enable only TDS reading
    wake();
    delay(500);
    EnableDisableSingleReading(EC, 0);
    EnableDisableSingleReading(TDS, 0);
    EnableDisableSingleReading(SG, 0);
    EnableDisableSingleReading(SAL, 0);
    EnableDisableSingleReading(TDS, 1);

    delay(600);
    ec.send_cmd("R");
    delay(600);
    ec.receive_cmd(ec_data, sizeof(ec_data));
    if (ec_data[0] != '\0') {
        *salinity = atof(ec_data);
        return true;
    }
    return false;
}

bool SalinitySensor::readEC(float* salinity){
    //enable only EC reading
    wake();
    EnableDisableSingleReading(SG, 0);
    EnableDisableSingleReading(SAL, 0);
    EnableDisableSingleReading(TDS, 0);
    EnableDisableSingleReading(EC, 0);
    EnableDisableSingleReading(EC, 1);

    delay(600);
    ec.send_cmd("R");
    delay(600);
    ec.receive_cmd(ec_data, sizeof(ec_data));
    sleep();
    if (ec_data[0] != '\0') {
        *salinity = atof(ec_data);
        return true;
    }
    return false;
}

void SalinitySensor::calibrate()    {

    wake();
    delay(300);
    Serial.println("Clearing previous calibration...");
    ec.send_cmd("Cal,clear");
    delay(300);
    // 2 Step Calibration Code 
    Serial.println("Starting Salnity Dry Calibration");
    ec.send_cmd("Cal,dry"); 
    delay(600);
    Serial.println("Finished, 10 seconds to switch to 12,880 low point calibration");
    delay(10000);
    ec.send_cmd("Cal,low,12880");
    delay(600);
    Serial.println("Finished low point, 10 seconds to switch to 80,000 high point calibration");
    delay(10000);
    ec.send_cmd("Cal,high,80000");
    delay(600);
    Serial.println("Salinity Calibration Complete!");


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


// Function to parse and store the 'O' parameter values
// void parseOParameters(const char* data, std::vector<std::pair<std::string, int>>& params) {
//     const char* found = strstr(data, "?O,");
//     if (found) {
//         found += 3; // Move past '?O,'
//         std::string oValues(found);
//         std::vector<std::string> tokens = split(oValues, ',');
//         for (size_t i = 0; i < tokens.size(); i++) {
//             std::string param = tokens[i];
//             if (i < tokens.size() - 1) {
//                 // The last token is the enable/disable value
//                 int enable = std::stoi(tokens[i + 1]);
//                 params.push_back(std::make_pair(param, enable));
//                 i++; // Skip the next token since it's already processed
//             }
//         }
//     }
// }


void SalinitySensor::sleep() {
    // ec.send_cmd("Sleep");
    // delay(300);
    //digitalWrite(EN_S, LOW);
    
    mcpGlobal.digitalWriteA(EN_S, LOW);

}  

void SalinitySensor::wake() {
    mcpGlobal.digitalWriteA(EN_S, HIGH);
}