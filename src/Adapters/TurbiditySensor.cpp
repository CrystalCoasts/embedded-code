// TurbiditySensor.cpp
#include "TurbiditySensor.h"
#include <EEPROM.h>
#include <Arduino.h>


TurbiditySensor& tbdty = TurbiditySensor::Get();
TurbiditySensor::TurbiditySensor() {}

void TurbiditySensor::begin() {
    // pinMode(BUTTON_CALIB, INPUT_PULLUP);
    EEPROM.get(EEPROM_VCLEAR_ADDRESS, vClear); // Use a specific EEPROM address for vClear
    analogReadResolution(12); //this is the default... but just in case
}

TurbiditySensor& TurbiditySensor::Get() {
    static TurbiditySensor instance;
    return instance;
}


/**
 * Allows to update the voltage reference for 'clear' water. 
 */
float TurbiditySensor::calibrate() {
    cumulativeRead = 0;
    for (int i = 0; i < READ_SAMPLES; ++i) {
        cumulativeRead += analogRead(T_ANALOG_PIN);
        delay(100); // Consider adjusting or removing delay based on needs
    }
    vClear = static_cast<float>(cumulativeRead) / READ_SAMPLES * (VREF/ ADC_DIGITAL); // Adjusting for 3.3V and 12-bit resolution
    EEPROM.put(EEPROM_VCLEAR_ADDRESS, vClear);
    Serial.print("Calibration complete. vClear set to: ");
    Serial.println(vClear);
    return vClear;
}


/**
 * @brief returns the turbidity reading in NTU (Nephelometric Turbidity Units)
 * 
 * Notes: 
 *  Sensor used for this had a 12 bit ADC and a max of 4050 NTU. It uses 5V
 *  ESP cannot handle 5V so voltage divider was neede to take it down to 3.3V
 */

float TurbiditySensor::readTurbidity() {
    int sensorValue = analogRead(T_ANALOG_PIN);
    float voltage = sensorValue * (VREF / ADC_DIGITAL);  // 3.3 V instead of 5
    float turbidityPercentage = (vClear - voltage) / vClear * MAX_NTU;  
    return turbidityPercentage; 
}
