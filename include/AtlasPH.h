#ifndef ATLASPH_H
#define ATLASPH_H

#include "Arduino.h"
#include <Wire.h>
#include <sequencer2.h>
#include <Ezo_i2c_util.h>
#include "ioExtender.h"

class AtlasPH   {

    public:
    float temperature; //float var used to hold the float value of the temperature.
    static AtlasPH& get();
    void begin();
    bool readTemp(float* ph, int temp);
    void sleep();
    void wake();
    static constexpr short EN_O = 0;  //used to turn off the circuit to save power (~2.9mA)

private:
    char computerdata[32];           //we make a 32 byte character array to hold incoming data from a pc/mac/other.
    byte received_from_computer = 0; //we need to know how many characters have been received.
    byte serial_event = 0;           //a flag to signal when data has been received from the pc/mac/other.
    byte code = 0;                   //used to hold the I2C response code.
    char temp_data[32];              //we make a 32 byte character array to hold incoming data from the D.O. circuit.
    byte in_char = 0;                //used as a 1 byte buffer to store inbound bytes from the D.O. Circuit.
    byte i = 0;                      //counter used for temp_data array.
    int time_ = 575;                 //used to change the delay needed depending on the command sent to the EZO Class DO Circuit.
    char *temp;                      //char pointer used in string parsing.

    String ecName = "PH";
    static constexpr short EC_ADDR = 0x63;
    Ezo_board ec = Ezo_board(EC_ADDR, ecName.c_str());
    char ec_data[32];

    bool parseValue(const char* rawBuff, char* parsedBuff, const char* key);

    //make singleton
    AtlasPH();     //private constructor
    AtlasPH(const AtlasPH&) = delete;     //prevent copying
    AtlasPH& operator=(const AtlasPH&) = delete;

};

extern AtlasPH& phGlobal;

#endif