#include "TurbiditySensor.h"
#include <EEPROM.h>
#include <Arduino.h>

TurbiditySensor& tbdty = TurbiditySensor::Get();

TurbiditySensor::TurbiditySensor() {}

void TurbiditySensor::begin() {
    EEPROM.get(EEPROM_VCLEAR_ADDRESS, vClear); // Use a specific EEPROM address for vClear
    analogReadResolution(12); // This is the default, but just in case
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
    vClear = static_cast<float>(cumulativeRead) / READ_SAMPLES * (VREF / ADC_DIGITAL); // Adjusting for 3.3V and 12-bit resolution
    EEPROM.put(EEPROM_VCLEAR_ADDRESS, vClear);
    Serial.print("Calibration complete. vClear set to: ");
    Serial.println(vClear);
    return vClear;
}

/**
 * @brief Returns the turbidity reading in NTU (Nephelometric Turbidity Units).
 * 
 * Notes: 
 *  Sensor used for this had a 12-bit ADC and a max of 4550 NTU. It uses 5V.
 *  ESP cannot handle 5V, so a voltage divider was needed to take it down to 3.3V.
 */
bool TurbiditySensor::readTurbidity(float* turbidity) {
    if (turbidity == nullptr) {
        return false; // Invalid pointer
    }
    
    int sensorValue = analogRead(T_ANALOG_PIN);
    float voltage = sensorValue * (VREF / ADC_DIGITAL); // Convert ADC value to voltage

    // Example validation, ensure voltage is within expected range
    if (voltage < 0.0 || voltage > VREF) {
        return false; // Voltage out of range
    }

    // Calculate the percentage of turbidity
    float turbidityPercentage = (vClear - voltage) / vClear * 100.0;

    // Conversion factor: 3.5% corresponds to 4550 NTU
    const float conversionFactor = 4550.0 / 3.5;

    // Calculate turbidity in NTU
    float turbidityNTU = turbidityPercentage * conversionFactor;

    // Ensure NTU is within 0 to MAX_NTU range
    if (turbidityNTU < 0.0) {
        turbidityNTU = 0.0;
    } else if (turbidityNTU > 4550.0) { // Assuming max NTU based on your sensor specs
        turbidityNTU = 4550.0;
    }

    // Assign the calculated value to the pointer
    *turbidity = turbidityNTU;
    
    return true; // Indicate successful reading
}
