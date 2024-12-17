#ifndef IOEXTENDER_H
#define IOEXTENDER_H

#include "Adafruit_MCP23X17.h"

class MCP   {
    public:
        void begin();
        void pinMode(int pin, uint8_t mode);
        void digitalWrite(uint8_t pin, uint8_t value);
        uint8_t digitalRead(uint8_t pin);
        static MCP& Get();

    private:
        
        Adafruit_MCP23X17 mcp;
        MCP();
        MCP(const MCP&) = delete;
        MCP& operator=(const MCP&) = delete;

};

extern MCP& mcpGlobal;

#endif
