#ifndef I2CADC_H
#define I2CADC_H

#include "Adafruit_ADS1X15.h"

class I2Cadc   {

    public:
        static I2Cadc& Get();
        void begin();
        float readADC(int pin);
        void readDifferential();
        void setGain(adsGain_t gain);

    private:

        //make singleton
        I2Cadc();   //private constructor
        I2Cadc(const I2Cadc&) = delete; //prevent cocpying
        I2Cadc& operator=(const I2Cadc&) = delete;
        Adafruit_ADS1015 adc1015;   

};

extern I2Cadc& i2cadc;  // Declaration of the global singleton instance

#endif