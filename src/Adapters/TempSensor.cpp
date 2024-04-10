#include "TempSensor.h"

TempSensor& temp = TempSensor::Get();


/**
 * Initializing the sensors instances for temperature and humidity
**/
TempSensor::TempSensor():
    dht(DHTPIN,DHTTYPE),
    oneWire(ONE_WIRE_BUS),
    sensors(&oneWire)
    {}

TempSensor& TempSensor::Get(){
    static TempSensor instance;
    return instance;
}

void TempSensor::begin(){
    dht.begin();
    sensors.begin();
}

float TempSensor::readHumidity(){
    return dht.readHumidity();
}

float TempSensor::readTemperature(TEMP tempScale){

    sensors.requestTemperatures();
    switch (tempScale)
    {
    case CELSIUS:
        return sensors.getTempCByIndex(TEMP_INDEX);
        break;

    case FARENHEIT:
        return sensors.getTempFByIndex(TEMP_INDEX);
        break;    
    default:
        return 0.0; //maybe have a better 'default'
        break;
    }    

}