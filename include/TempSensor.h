#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H
#include <DHT.h>

#include <OneWire.h>
#include <DallasTemperature.h>

constexpr uint8_t ONE_WIRE_BUS = 4;
constexpr uint8_t DHTPIN = 14;
constexpr uint8_t DHTTYPE = DHT11; // change to actual dht
constexpr uint8_t TEMP_INDEX = 0;

enum TEMP {CELSIUS,FARENHEIT};


class TempSensor
{
public:
    static TempSensor& Get();
    void begin();
    float readTemperature(TEMP tempScale);
    float readHumidity();

private:
    TempSensor();
    DHT dht;
    OneWire oneWire;
    DallasTemperature sensors;
    
    //making it signletton
    TempSensor (const TempSensor&) = delete;
    TempSensor& operator=(const TempSensor&)=delete; 

};

extern TempSensor& temp;

#endif