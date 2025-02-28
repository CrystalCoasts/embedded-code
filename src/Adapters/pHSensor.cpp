#include "pHSensor.h"

pHSensor& phGloabl = pHSensor::Get();

pHSensor::pHSensor() {};

bool pHSensor::begin()  {
    //pinMode(EN, OUTPUT);
    //digitalWrite(EN, HIGH);
    mcpGlobal.pinModeA(EN,0);
    wakeup();
    if(pH.begin())  {
        return true;
    }
    return false;
    sleep();
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
    mcpGlobal.digitalWriteA(EN,HIGH);
    //digitalWrite(EN, HIGH);
    delay(200);
}
void pHSensor::sleep() {
    mcpGlobal.digitalWriteA(EN, LOW);
    delay(200);
}

void pHSensor::cal_mid()  {
    pH.cal_mid();
}
void pHSensor::cal_low()  {
    pH.cal_low();
}
void pHSensor::cal_high() {
    pH.cal_high();

}
void pHSensor::cal_clear()    {
    pH.cal_clear();
}

