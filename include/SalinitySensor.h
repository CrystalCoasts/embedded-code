#ifndef SALINITY_SENSOR_H
#define SALINITY_SENSOR_H

#include <Ezo_i2c.h>
#include <Wire.h>
#include <sequencer2.h>
#include <Ezo_i2c_util.h>
#include "ioExtender.h"

//data reading options
enum EC {SAL, TDS, EC, SG};

class SalinitySensor {
public:
    static SalinitySensor& Get();
    void begin();
    void EnableDisableSingleReading(uint8_t readOption, uint8_t data);
    bool readSalinity(float* salinity);
    bool readTDS(float* tds);
    bool readEC(float* ec);
    void DisableAllReadings();
    void calibrate();
    void sleep();
    void wake();
    static constexpr short EN_S = 0;    //EN pin to sleep device

private:
    String ecName = "SAL";
    static constexpr short EC_ADDR = 0x14;
    Ezo_board ec = Ezo_board(EC_ADDR, ecName.c_str());
    char ec_data[32];       //used to store incoming i2c data from salinity sensor

    static constexpr uint8_t TEMP_COMP = 25;        //assumed 25 degrees celcius average for data measurement
    static constexpr uint8_t K_FACTOR = 1;          //Using type 1 conductivity sensor from Atlas
    bool salFlag = false;
    bool tdsFlag = false;

    bool parseValue(const char* rawBuff, char* parsedBuff, const char* key);

    //make singleton
    SalinitySensor();   //private constructor
    SalinitySensor(const SalinitySensor&) = delete; //prevent copying
    SalinitySensor& operator=(const SalinitySensor&) = delete;
};

extern SalinitySensor& sal;     // Declaration of the global singleton instance

#endif // SALINITY_SENSOR_H
