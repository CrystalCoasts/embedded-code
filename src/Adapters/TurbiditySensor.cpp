#include "TurbiditySensor.h"
#include <Arduino.h>

TurbiditySensor& tbdty = TurbiditySensor::Get();

TurbiditySensor::TurbiditySensor() {}

void TurbiditySensor::begin() {
    EEPROM.get(EEPROM_VCLEAR_ADDRESS, vClear); // Retrieve the vClear from EEPROM
    analogReadResolution(12); // Set ADC resolution to 12-bit
}

TurbiditySensor& TurbiditySensor::Get() {
    static TurbiditySensor instance;
    return instance;
}

float TurbiditySensor::calibrate() {
    cumulativeRead = 0;
    for (int i = 0; i < READ_SAMPLES; ++i) {
        cumulativeRead += analogRead(T_ANALOG_PIN);
        delay(100); // Delay for stability
    }
    float sensorVoltage = static_cast<float>(cumulativeRead) / READ_SAMPLES * (VREF / ADC_DIGITAL);
    vClear = sensorVoltage * DIVIDER_RATIO; // Adjust for voltage divider
    EEPROM.put(EEPROM_VCLEAR_ADDRESS, vClear);
    Serial.print("Calibration complete. vClear set to: ");
    Serial.println(vClear);
    return vClear;
}

bool TurbiditySensor::readTurbidity(float* turbidity) {
    if (turbidity == nullptr) {
        return false; // Invalid pointer
    }

    int sensorValue = analogRead(T_ANALOG_PIN);
    float Vout = sensorValue * (VREF / ADC_DIGITAL); // Convert ADC value to voltage
    float Vin = Vout * DIVIDER_RATIO; // Adjust for voltage divider

    
    if (Vin < 0.0 || Vin > 5.0) {
        return false; // Voltage out of range
    }

    // Calculate the percentage of turbidity
    float turbidityPercentage = (vClear - Vin) / vClear * 100.0;

    // Conversion factor: 3.5% corresponds to 4550 NTU (from keystudio website)
    const float conversionFactor = MAX_NTU / 3.5;

    // Calculate turbidity in NTU
    float turbidityNTU = turbidityPercentage * conversionFactor;

    // Ensure NTU is within 0 to MAX_NTU range
    if (turbidityNTU < 0.0) {
        turbidityNTU = 0.0;
    } else if (turbidityNTU > MAX_NTU) {
        turbidityNTU = MAX_NTU;
    }

    // Assign the calculated value to the pointer
    *turbidity = turbidityNTU;

    return true; // Indicate successful reading
}
