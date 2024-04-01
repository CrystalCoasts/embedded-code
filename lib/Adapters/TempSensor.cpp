#include "TempSensor.h"
#include "config.h"

TempSensor::TempSensor() : dht(DHTPIN, DHTTYPE) {}

TempSensor& TempSensor::Get() {
    static TempSensor instance; // Guaranteed to be created only once
    return instance;
}

void TempSensor::begin() {
    dht.begin();
}

float TempSensor::readTemperature() {
    return dht.readTemperature();
}

float TempSensor::readHumidity(){
    return dht.readHumidity();
}
