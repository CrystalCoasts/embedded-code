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


float I2Cadc::readADC(int pin){     //reads adc single time
    return adc1015.readADC_SingleEnded(pin);
}

/*
    Gain can be set through the below method. The following gains are:
    - GAIN_TWOTHIRDS (for an input range of +/- 6.144V)
    - GAIN_ONE (for an input range of +/-4.096V)
    - GAIN_TWO (for an input range of +/-2.048V)
    - GAIN_FOUR (for an input range of +/-1.024V)
    - GAIN_EIGHT (for an input range of +/-0.512V)
    - GAIN_SIXTEEN (for an input range of +/-0.256V)
*/

void I2Cadc::setGain(adsGain_t gain)    {       //sets gain
    adc1015.setGain(gain);
}




