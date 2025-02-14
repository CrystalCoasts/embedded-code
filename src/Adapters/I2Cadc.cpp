#include "I2Cadc.h"

I2Cadc& i2cadc = I2Cadc::Get();

I2Cadc::I2Cadc()    {};

I2Cadc& I2Cadc::Get()  {
    static I2Cadc instance;
    return instance;
}

void I2Cadc::begin()    {
    adc1015.begin();
}

float I2Cadc::readADC(int pin){
    return adc1015.readADC_SingleEnded(pin);
}

void I2Cadc::setGain(adsGain_t gain)    {
    adc1015.setGain(gain);
}




