#ifndef ATLAS_TEMP_H
#define ATLAS_TEMP_H

#include <Ezo_i2c.h>
#include <Wire.h>
#include <sequencer2.h>
#include <Ezo_i2c_util.h>
#include "ioExtender.h"

class AtlasTemp {
public:
    float temperature; // Variable to hold the temperature value
    static AtlasTemp& Get(); // Singleton instance getter
    void begin(); // Initialize the sensor
    bool readTemperature(float* temp); // Read temperature and store in provided pointer
    void sleep(); // Put the sensor to sleep
    void wake(); // Wake the sensor up
    static constexpr short EN = 1; // Used to turn off the circuit to save power (~2.9mA)
    
private:

    String ecName = "TEMP";
    static constexpr short EC_ADDR = 0x66;
    Ezo_board ezoTemp = Ezo_board(EC_ADDR, ecName.c_str());

    char computerdata[32]; // Buffer for incoming data from a PC/Mac/other
    AtlasTemp(); // Private constructor for singleton
    AtlasTemp(const AtlasTemp&) = delete; // Prevent copying
    AtlasTemp& operator=(const AtlasTemp&) = delete; // Prevent assignment

};

extern AtlasTemp& atlasTempSensor;

#endif