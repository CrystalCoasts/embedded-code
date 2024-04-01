// TurbiditySensor.cpp
#include "turbiditySensor.h"
#include <EEPROM.h>
#include <Arduino.h>

TurbiditySensor::TurbiditySensor() {}

void TurbiditySensor::begin() {
    pinMode(BUTTON_CALIB, INPUT_PULLUP);
    EEPROM.get(EEPROM_VCLEAR_ADDRESS, vClear); // Use a specific EEPROM address for vClear
}

void TurbiditySensor::calibrate() {
    cumulativeRead = 0;
    for (int i = 0; i < READ_SAMPLES; ++i) {
        cumulativeRead += analogRead(T_ANALOG_PIN);
        delay(10);
    }
    vClear = static_cast<float>(cumulativeRead) / READ_SAMPLES * (5.0 / 1023.0);
    EEPROM.put(EEPROM_VCLEAR_ADDRESS, vClear);
}

float TurbiditySensor::readTurbidity() {
    if (digitalRead(BUTTON_CALIB) == LOW) { // Assuming LOW means button pressed
        calibrate();
        return -1.0; // Indicate calibration mode
    }
    cumulativeRead = 0;
    for (int i = 0; i < READ_SAMPLES; ++i) {
        cumulativeRead += analogRead(T_ANALOG_PIN);
        delay(10);
    }
    unsigned int avgSensorRead = cumulativeRead / READ_SAMPLES;
    float voltage = avgSensorRead * (5.0 / 1023.0);
    return 100.0 - (voltage / vClear) * 100.0;
}
