// TurbiditySensor.cpp
#include "TurbiditySensor.h"
#include <EEPROM.h>
#include <Arduino.h>


TurbiditySensor& tbdty = TurbiditySensor::Get();
TurbiditySensor::TurbiditySensor() {}

void TurbiditySensor::begin() {
    // pinMode(BUTTON_CALIB, INPUT_PULLUP);
    EEPROM.get(EEPROM_VCLEAR_ADDRESS, vClear); // Use a specific EEPROM address for vClear
}

void TurbiditySensor::calibrate() {
    cumulativeRead = 0;
    for (int i = 0; i < READ_SAMPLES; ++i) {
        cumulativeRead += analogRead(T_ANALOG_PIN);
        delay(100);
    }
    vClear = static_cast<float>(cumulativeRead) / READ_SAMPLES * (5.0 / 1023.0);
    EEPROM.put(EEPROM_VCLEAR_ADDRESS, vClear);
    Serial.print("Calibration complete. vClear set to: ");
    Serial.println(vClear);
}

float TurbiditySensor::readTurbidity() {
    int sensorValue = analogRead(T_ANALOG_PIN);
    float voltage = sensorValue * (5.0 / 1023.0);  // Convert the analog reading to voltage
    float turbidityPercentage = (vClear - voltage) / vClear * 100.0;  // Calculate the percentage of turbidity
    return turbidityPercentage;  // Return the turbidity as a percentage of decrease from clear water
}
