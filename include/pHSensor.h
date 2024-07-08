#ifndef PHSENSOR_H
#define PHSENSOR_H

#include "config.h"
//#include "driver/adc.h"
#include "ph_surveyor.h"
#include "Arduino.h" 

//ADC1 CONFIG IS DONE IN ph_surveyor.h + ph_surveyor.cpp
//THIS IS ONLY HELPER CLASS TO INSTANTIATE 1 INSTANCE

class pHSensor {
public:
    static constexpr uint8_t EEPROM_VCLEAR_ADDRESS = 0;
    static constexpr uint8_t PIN = 35;
    static constexpr uint8_t EN = 14;

    static pHSensor& Get();
    void begin();
    bool readpH(float* pHValue);

private:

    unsigned int cumulativeRead = 0;

    Surveyor_pH pH = Surveyor_pH(PIN);
    pHSensor();  // Private constructor
    pHSensor(const pHSensor&) = delete; // Prevent copying
    pHSensor& operator=(const pHSensor&) = delete;
};

extern pHSensor& phGloabl;  // Declaration of the global singleton instance

#endif // TURBIDITY_SENSOR_H
