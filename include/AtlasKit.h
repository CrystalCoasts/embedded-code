#ifndef WATER_QUALITY_SENSOR_H
#define WATER_QUALITY_SENSOR_H

// #include <Arduino.h>
#include <Wire.h>

enum class MType {
    EC,     // Electrical Conductivity
    TDS,    // Total Dissolved Solids
    SAL,    // Salinity
    SG      // Specific Gravity
};

class WaterQualitySensor {
public:

    static constexpr uint8_t EZO_CIRCUIT_I2C_ADDRESS = 0x3C;

    static WaterQualitySensor& Get();
    void begin();
    float read(MType type);
    uint8_t sendCommand(const String& command);
    String readResponse();
    float parseValue(const char* data);

private:
    private:
    WaterQualitySensor();
    WaterQualitySensor(const WaterQualitySensor&) = delete;
    WaterQualitySensor& operator=(const WaterQualitySensor&) = delete;
};

extern WaterQualitySensor& qSensor;

#endif // WATER_QUALITY_SENSOR_H
