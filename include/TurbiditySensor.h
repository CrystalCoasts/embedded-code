#ifndef TURBIDITY_SENSOR_H
#define TURBIDITY_SENSOR_H

#include <EEPROM.h>
#include "config.h"

class TurbiditySensor {
public:
    static constexpr short READ_SAMPLES = 25;
    static constexpr short T_ANALOG_PIN = 32; // ADC1 pin
    static constexpr uint8_t EEPROM_VCLEAR_ADDRESS = 0;

    static TurbiditySensor& Get();
    void begin();
    float calibrate();
    bool readTurbidity(float* turbidity);

private:
    static constexpr float VREF = 3.3;
    static constexpr short MAX_NTU = 4550;
    static constexpr short ADC_DIGITAL = 4095;
    //vout = vin*(R2/(R1+R2)) -> vin20/(10+20) -> vin = vou*(3/2)
    static constexpr float DIVIDER_RATIO = 3.0 / 2.0; 

    unsigned int cumulativeRead = 0;
    float vClear = 2.82; // Default calibration value

    TurbiditySensor();  // Private constructor
    TurbiditySensor(const TurbiditySensor&) = delete; // Prevent copying
    TurbiditySensor& operator=(const TurbiditySensor&) = delete;
};

extern TurbiditySensor& tbdty;  // Declaration of the global singleton instance

#endif // TURBIDITY_SENSOR_H
