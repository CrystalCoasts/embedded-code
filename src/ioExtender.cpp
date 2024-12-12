#include <ioExtender.h>

MCP& mcpGlobal = MCP::Get();

MCP::MCP() {};

void MCP::begin()   {
    if(!mcp.begin_I2C(0x20))    {
        Serial.println("Error, MCP did not connect to I2C");
    }else
        Serial.println("I2C MCP connected!");
}

void MCP::pinMode(int pin, uint8_t mode)    {
    mcp.pinMode(pin, mode);
}

void MCP::digitalWrite(uint8_t pin, uint8_t value)  {
    mcp.digitalWrite(pin, value);
}

uint8_t MCP::digitalRead(uint8_t pin)    {
    return mcp.digitalRead(pin);
}

MCP& MCP::Get() {
    static MCP instance;
    return instance;
}

