#include "pHSensor.h"

pHSensor& phGloabl = pHSensor::Get();

pHSensor::pHSensor() {};

bool pHSensor::begin()  {
    mcpGlobal.pinModeA(EN,0);   //Setup EN pin for output
    wakeup();       //Turn on pH sensor
    if(pH.begin())  {
        return true;
    }
    return false;
    sleep();        //turn off pH sensor
}

bool pHSensor::readpH(float* pHValue) {
    wakeup();       // turn on pH sensor
    if(pHValue == nullptr)  {       //if pointer is empty, return false
        return false;
    }

    
    float p = pH.read_ph();     //read pH
    if(isnan(p))   {
        return false;
    }
    sleep();        //turn off pH sensor
    *pHValue = p;
    return pHValue;
}

pHSensor& pHSensor::Get()    {
    static pHSensor instance;
    return instance; 
}

//Power Consumption Methods
void pHSensor::wakeup() {
    mcpGlobal.digitalWriteA(EN,HIGH);       //outputs high value to bjt switch to turn on pH sensor
    //digitalWrite(EN, HIGH);
    delay(200);
}
void pHSensor::sleep() {
    mcpGlobal.digitalWriteA(EN, LOW);       //outputs low value to bjt switch to turn off pH sensor
    delay(200);
}

//Calibration Methods
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

