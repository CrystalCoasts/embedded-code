#include "AtlasKit.h"

WaterQualitySensor& qSensor = WaterQualitySensor::Get();

WaterQualitySensor& WaterQualitySensor::Get() {
    static WaterQualitySensor instance;
    return instance;
}

WaterQualitySensor::WaterQualitySensor() {}

void WaterQualitySensor::begin() {
    Wire.begin();
}

uint8_t WaterQualitySensor::sendCommand(const String& command) {
    Wire.beginTransmission(EZO_CIRCUIT_I2C_ADDRESS);
    Wire.write(command.c_str());
    uint8_t result = Wire.endTransmission();
    delay(command[0] == 'R' ? 570 : 300); // Delay depends on command type
    return result == 0; // Return true if transmission was successful
}

String WaterQualitySensor::readResponse() {
    Wire.requestFrom((int)EZO_CIRCUIT_I2C_ADDRESS, 32);
    String response = "";
    while (Wire.available()) {
        char c = Wire.read();
        response += c;
        if (c == 0) break;
    }
    return response;
}


float WaterQualitySensor::read(MType type) {
    if (!sendCommand("R")) {
        return NAN;  // Return NaN if command sending failed
    }
    String response = readResponse();
    if (response.isEmpty()) {
        return NAN;  // Return NaN if response is empty
    }
    char buffer[response.length() + 1];
    response.toCharArray(buffer, sizeof(buffer));
    char* data = strtok(buffer, ",");
    int index = static_cast<int>(type);

    for (int i = 0; i < index && data != NULL; i++) {
        data = strtok(NULL, ",");
    }

    return parseValue(data);
}


float WaterQualitySensor::parseValue(const char* data) {
    if (!data) return NAN; // Return NaN if data is null
    return atof(data);
}