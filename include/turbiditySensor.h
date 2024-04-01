// TurbiditySensor.h
#ifndef TURBIDITY_SENSOR_H
#define TURBIDITY_SENSOR_H

#include <EEPROM.h>
#include "config.h"


// //button for turbidity sensor settings
constexpr short BUTTON_CALIB = 2; //change pin
constexpr short READ_SAMPLES = 25; // Number of samples for averaging
constexpr short T_ANALOG_PIN = 1;
constexpr int EEPROM_VCLEAR_ADDRESS = 0;

class TurbiditySensor {
public:
    static TurbiditySensor& Get() {
        static TurbiditySensor instance;
        return instance;
    }

    void begin();
    void calibrate();
    float readTurbidity();

private:
    TurbiditySensor(); // Private constructor
    unsigned int cumulativeRead = 0;
    float vClear = 2.85; // Default calibration value
    static constexpr short READ_SAMPLES = 25;

    // Deleted copy constructor and assignment operator to prevent copying
    TurbiditySensor(const TurbiditySensor&) = delete;
    TurbiditySensor& operator=(const TurbiditySensor&) = delete;
};

#endif // TURBIDITY_SENSOR_H
