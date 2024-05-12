#ifndef SALINITY_SENSOR_H
#define SALINITY_SENSOR_H

#include <Ezo_i2c.h>
#include <Wire.h>
#include <sequencer2.h>
#include <Ezo_i2c_util.h>

enum EC {SAL, TDS, EC, SG};

class SalinitySensor {
public:
    static SalinitySensor& Get();
    void begin();
    void EnableDisableSingleReading(uint8_t readOption, uint8_t data);
    bool readSalinity(float* salinity);
    void DisableAllReadings();

private:
    String ecName = "SAL";
    static constexpr short EC_ADDR = 0x14;
    Ezo_board ec = Ezo_board(EC_ADDR, ecName.c_str());
    char ec_data[32];

    static constexpr uint8_t TEMP_COMP = 25;
    static constexpr uint8_t K_FACTOR = 1;
    bool salFlag = false;

    bool parseValue(const char* rawBuff, char* parsedBuff, const char* key);
    SalinitySensor();
    SalinitySensor(const SalinitySensor&) = delete;
    SalinitySensor& operator=(const SalinitySensor&) = delete;
};

extern SalinitySensor& sal;

#endif // SALINITY_SENSOR_H
