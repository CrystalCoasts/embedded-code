#ifndef TURBIDITY_SENSOR_H
#define TURBIDITY_SENSOR_H

#include <EEPROM.h>
#include "config.h"



class TurbiditySensor {
public:

    // constexpr short BUTTON_CALIB = 2;
    static constexpr short READ_SAMPLES = 25;
    static constexpr short T_ANALOG_PIN = 36; // we need one of the adc1 pins, adc2 pins cannot be used while wifi is in use
    static constexpr uint8_t EEPROM_VCLEAR_ADDRESS = 0;

    static TurbiditySensor& Get();
    void begin();
    void calibrate();
    float readTurbidity();

private:
    TurbiditySensor();  // Private constructor
    unsigned int cumulativeRead = 0;
    float vClear = 15; // Default calibration value

    // Deleted copy constructor and assignment operator to prevent copying
    TurbiditySensor(const TurbiditySensor&) = delete;
    TurbiditySensor& operator=(const TurbiditySensor&) = delete;
};

extern TurbiditySensor& tbdty;  // Declaration of the global singleton instance

#endif // TURBIDITY_SENSOR_H
