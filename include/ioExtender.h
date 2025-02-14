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
        MCP();
        MCP(const MCP&) = delete;
        MCP& operator=(const MCP&) = delete;

};

extern MCP& mcpGlobal;

#endif
