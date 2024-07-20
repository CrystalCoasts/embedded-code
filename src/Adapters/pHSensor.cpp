#include "pHSensor.h"

pHSensor& phGloabl = pHSensor::Get();

pHSensor::pHSensor() {};

void pHSensor::begin()  {
    pH.begin();
    pinMode(EN, OUTPUT);
    wakeup();
}

bool pHSensor::readpH(float* pHValue) {
    wakeup();
    if(pHValue == nullptr)  {
        return false;
    }

    
    float p = pH.read_ph();
    if(isnan(p))   {
        return false;
    }
    sleep();
    *pHValue = p;
    return pHValue;
}

pHSensor& pHSensor::Get()    {
    static pHSensor instance;
    return instance; 
}

void pHSensor::wakeup() {
    digitalWrite(EN, HIGH);
    delay(1000);
}
void pHSensor::sleep() {
    digitalWrite(EN, LOW);
    delay(1000);
}

