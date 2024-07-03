#include "SalinitySensor.h"
#include <iostream>

SalinitySensor& sal = SalinitySensor::Get();

SalinitySensor::SalinitySensor() {}

void SalinitySensor::begin() {
    bool calibrated, tempComp, kFactor;
    char parsedData[32];
    delay(500);

    ec.send_cmd("Cal,?");
    delay(500);
    ec.receive_cmd(ec_data, sizeof(ec_data));

    if (parseValue(ec_data, parsedData, "?Cal")) {
        Serial.println("Calibration profile detected.");
        calibrated = (atoi(parsedData) != 0);
    }

    ec.send_cmd("T,?");
    delay(500);
    ec.receive_cmd(ec_data, sizeof(ec_data));

    if (parseValue(ec_data, parsedData, "?T")) {
        Serial.println("Correct Temp compensation.");
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
        Serial.println("Correct Conversion Factor.");
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
    switch (readOption) {
    case SAL:
        command += "S," + String(data);
        Serial.println("Salinity reading enabled");
        salFlag = true;
        break;
    case EC:
        command += "EC," + String(data);
        break;
    case TDS:
        command += "TDS," + String(data);
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
    ec.send_cmd("O,?");
    delay(300);
    ec.receive_cmd(ec_data, sizeof(ec_data));
    char salStr[32];
    if (!salFlag) {
        EnableDisableSingleReading(TDS, 1);
    }

    ec.send_cmd("R");
    delay(600);
    ec.receive_cmd(ec_data, sizeof(ec_data));
    // Serial.print("RAW salinity Read: ");
    // Serial.println(ec_data);

    if (ec_data[0] != '\0') {
        *salinity = atof(ec_data);
        return true;
    }
    return false;
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
