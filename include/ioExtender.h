/*
    New library for I2C IO extender MCP23017. The prewritten one did not work properly, so I created my own functions

    Using the register values and the data sheet, I wrote code to read and write on both A and B bus.
*/

#ifndef IOEXTENDER_H
#define IOEXTENDER_H

#define ADDR    0x20
#define IODIRA  0x00
#define IODIRB  0x01
#define GPIOA   0x12
#define GPIOB   0x13
#define OLATA   0x14
#define OLATB   0x15

#include "Adafruit_MCP23X17.h"


class MCP   {
    public:
        void begin();
        void pinModeA(int pin, uint8_t mode);
        void pinModeB(int pin, uint8_t mode);
        void digitalWriteA(uint8_t pin, uint8_t value);
        void digitalWriteB(uint8_t pin, uint8_t value);
        uint8_t readPinStateA();
        uint8_t readPinStateB();
        uint8_t readDirA();
        uint8_t readDirB();
        uint8_t digitalReadA(uint8_t pin);
        uint8_t digitalReadB(uint8_t pin);
        static MCP& Get();

    private:
        
        Adafruit_MCP23X17 mcp;

        // make singleton
        MCP();  //private constructor
        MCP(const MCP&) = delete;   //prevent copying
        MCP& operator=(const MCP&) = delete;

};

extern MCP& mcpGlobal;  // Declaration of the global singleton instance

#endif
