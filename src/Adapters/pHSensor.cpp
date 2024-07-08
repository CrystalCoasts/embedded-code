#include "pHSensor.h"

pHSensor& phGloabl = pHSensor::Get();

pHSensor::pHSensor() {};

void pHSensor::begin()  {
    pinMode(EN, OUTPUT);
    digitalWrite(EN,HIGH);
    pH.begin();
}

bool pHSensor::readpH(float* pHValue) {
    digitalWrite(EN,HIGH);
    delay(1000);
    if(pHValue == nullptr)  {
        return false;
    }
    float p = pH.read_ph();
    if(isnan(p))   {
        return false;
    }

    digitalWrite(EN,LOW);

    *pHValue = p;
    return pHValue;
}

pHSensor& pHSensor::Get()    {
    static pHSensor instance;
    return instance; 
}


