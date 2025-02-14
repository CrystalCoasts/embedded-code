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
        I2Cadc();
        I2Cadc(const I2Cadc&) = delete;
        I2Cadc& operator=(const I2Cadc&) = delete;
        Adafruit_ADS1015 adc1015;   

};

extern I2Cadc& i2cadc;

#endif