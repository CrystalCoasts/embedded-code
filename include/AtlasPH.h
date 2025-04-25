#ifndef ATLAS_PH_H
#define ATLAS_PH_H

#include <Ezo_i2c.h>
#include <Wire.h>
#include <sequencer2.h>
#include <Ezo_i2c_util.h>
#include "ioExtender.h"

class AtlasPH {
public:
    float ph; // Variable to hold the temperature value
    static AtlasPH& Get(); // Singleton instance getter
    void begin(); // Initialize the sensor
    bool readPH(float* ph, float temp); // Read temperature and store in provided pointer
    void sleep(); // Put the sensor to sleep
    void wake(); // Wake the sensor up
    static constexpr short EN = 1; // Used to turn off the circuit to save power (~2.9mA)
    
private:

    String ecName = "PH";
    static constexpr short EC_ADDR = 0x63;
    Ezo_board ezoPH = Ezo_board(EC_ADDR, ecName.c_str());

    char computerdata[32]; // Buffer for incoming data from a PC/Mac/other
    AtlasPH(); // Private constructor for singleton
    AtlasPH(const AtlasPH&) = delete; // Prevent copying
    AtlasPH& operator=(const AtlasPH&) = delete; // Prevent assignment

};

extern AtlasPH& atlasPHSensor;

#endif