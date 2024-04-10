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

void WaterQualitySensor::sendCommand(const String& command) {
    Wire.beginTransmission(EZO_CIRCUIT_I2C_ADDRESS);
    Wire.write(command.c_str());
    Wire.endTransmission();
    delay(command[0] == 'R' ? 570 : 300); // Delay depends on command type
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
    sendCommand("R");  // Send the read command
    String response = readResponse();
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