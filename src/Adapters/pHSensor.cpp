#include "pHSensor.h"

pHSensor& phGloabl = pHSensor::Get();

pHSensor::pHSensor() {};

void pHSensor::begin()  {
    pH.begin();
}

bool pHSensor::readpH(float* pHValue) {
    if(pHValue == nullptr)  {
        return false;
    }
    float p = pH.read_ph();
    if(isnan(p))   {
        return false;
    }

    *pHValue = p;
    return pHValue;
}

pHSensor& pHSensor::Get()    {
    static pHSensor instance;
    return instance; 
}


