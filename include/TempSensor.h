#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H
#include <DHT.h>

#include <OneWire.h>
//#include <DallasTemperature.h>
#include "dallasTemperature.h"



enum TEMP {CELSIUS,FAHRENHEIT};


class TempSensor
{
public:
    //Initializing pins and modes
    static constexpr uint8_t ONE_WIRE_BUS = 33;
    static constexpr uint8_t DHTPIN = 32;
    static constexpr uint8_t DHTTYPE = DHT22;
    static constexpr uint8_t TEMP_INDEX = 0;


    static TempSensor& Get();
    void begin();
    bool readTemperature(TEMP tempScale, float* temperature);
    bool readTemp(TEMP tempScale, float* temperature);
    bool readHumidity(float* humidity);
    int findDevices();



private:
    DHT dht;
    OneWire oneWire;
    DallasTemperature sensors;    

    //making it singleton
    TempSensor();   //private constructor
    TempSensor (const TempSensor&) = delete;    //prevent copying
    TempSensor& operator=(const TempSensor&)=delete; 

};

extern TempSensor& temp;        // Declaration of the global singleton instance

#endif