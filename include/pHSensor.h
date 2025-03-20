#ifndef PHSENSOR_H
#define PHSENSOR_H

#include "config.h"
//#include "driver/adc.h"
#include "ph_surveyor.h"
#include "Arduino.h"
#include "I2Cadc.h"
#include "ioExtender.h"

//ADC1 CONFIG IS DONE IN ph_surveyor.h + ph_surveyor.cpp
//THIS IS ONLY HELPER CLASS TO INSTANTIATE 1 INSTANCE

class pHSensor {
public:
    static constexpr uint8_t EEPROM_VCLEAR_ADDRESS = 0;
    static constexpr uint8_t PIN = 0;   //I2C ADC
    //static constexpr uint8_t PIN = 39;    //ESP32 ONBOARD
    static constexpr uint8_t EN = 0;

    static pHSensor& Get();
    bool begin();
    bool readpH(float* pHValue);
    void sleep();
    void wakeup();
    void cal_mid();
    void cal_low();
    void cal_high();
    void cal_clear();

private:

    unsigned int cumulativeRead = 0;

    Surveyor_pH pH = Surveyor_pH(PIN);
    
    //make singleton
    pHSensor();  // Private constructor
    pHSensor(const pHSensor&) = delete; // Prevent copying
    pHSensor& operator=(const pHSensor&) = delete;
};

extern pHSensor& phGloabl;  // Declaration of the global singleton instance

#endif // TURBIDITY_SENSOR_H
