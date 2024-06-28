#ifndef DOSENSOR_H
#define DOSENSOR_H


#include <Ezo_i2c.h>
#include <Wire.h>
#include <sequencer2.h>
#include <Ezo_i2c_util.h>

#define urmom 0x97

class DOSensor    {

    public:
        float DO_float;                  //float var used to hold the float value of the D.O.
        float sat_float;                 //float var used to hold the float value of the percent saturation.
        static DOSensor& get();
        void begin();
        bool readDO(float* DO, float salinity, float temp);


    private:
        char computerdata[32];           //we make a 32 byte character array to hold incoming data from a pc/mac/other.
        byte received_from_computer = 0; //we need to know how many characters have been received.
        byte serial_event = 0;           //a flag to signal when data has been received from the pc/mac/other.
        byte code = 0;                   //used to hold the I2C response code.
        char do_data[32];                //we make a 32 byte character array to hold incoming data from the D.O. circuit.
        byte in_char = 0;                //used as a 1 byte buffer to store inbound bytes from the D.O. Circuit.
        byte i = 0;                      //counter used for DO_data array.
        int time_ = 575;                 //used to change the delay needed depending on the command sent to the EZO Class DO Circuit.
        char *DO;                        //char pointer used in string parsing.
        char *sat;                       //char pointer used in string parsing.

        static constexpr uint8_t TEMP_COMP = 25;
        static constexpr uint8_t SAL_COMP = 37;
        String ecName = "DO";
        static constexpr short EC_ADDR = 0x61;
        Ezo_board ec2 = Ezo_board(EC_ADDR, ecName.c_str());
        char ec_data[32];

        bool parseValue(const char* rawBuff, char* parsedBuff, const char* key);
        DOSensor();
        DOSensor(const DOSensor&) = delete;
        DOSensor& operator=(const DOSensor&) = delete;

};

extern DOSensor& DO;


#endif