#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <DHT.h>

class TempSensor {

#define DHTPIN 14
#define DHTTYPE DHT11 // Assuming DHT11 for simplicity

public:
    static TempSensor& Get();
    void begin();
    float readTemperature();
    float readHumidity();
private:
    DHT dht;
    TempSensor();

    //making it signletton
    TempSensor (const TempSensor&) = delete;
    TempSensor& operator=(const TempSensor&)=delete; 
};

#endif
