#include <ioExtender.h>

MCP& mcpGlobal = MCP::Get();

MCP::MCP() {};

void MCP::begin()   {
    if(!mcp.begin_I2C())    {
        Serial.println("Error, MCP did not connect to I2C");
    }else
        Serial.println("I2C MCP connected!");
}

uint8_t MCP::readPinStateA()   {
    Wire.beginTransmission(ADDR);
    Wire.write(GPIOA);
    Wire.endTransmission();

    Wire.requestFrom(ADDR, (uint8_t) 1);
    if(Wire.available())    {
        uint8_t out = Wire.read();
        Serial.println(out);
        return out;
    }
    return 0;
}

uint8_t MCP::readPinStateB()    {
    Wire.beginTransmission(ADDR);
    Wire.write(GPIOB);
    Wire.endTransmission();

    Wire.requestFrom(ADDR, (uint8_t) 1);
    if(Wire.available())    {
        uint8_t out = Wire.read();
        Serial.println(out);
        Wire.endTransmission();
        delay(200);
        return out;   
    }
    Wire.endTransmission();
    return 0;
}

uint8_t MCP::readDirA()  {
    Wire.beginTransmission(ADDR);
    Wire.write(IODIRA);
    Wire.endTransmission();
    Wire.requestFrom(ADDR, (uint8_t) 1);
    if(Wire.available())  {
        uint8_t out = Wire.read();
        Serial.println(out);
        return out;    }
    return 0;
}

uint8_t MCP::readDirB()  {
    Wire.beginTransmission(ADDR);
    Wire.write(IODIRB);
    Wire.endTransmission();
    Wire.requestFrom(ADDR, (uint8_t) 1);
    if(Wire.available())  {
        uint8_t out = Wire.read();
        Serial.println(out);
        return out;    }
    return 0;
}

void MCP::pinModeA(int pin, uint8_t mode)    {   
    int8_t current = readDirA();
    if (mode == INPUT) {
        current |= (1 << pin);  // Set bit to 1 for input
    } else {
        current &= ~(1 << pin); // Clear bit to 0 for output
    }
    Wire.beginTransmission(ADDR);
    Wire.write(IODIRA);
    Wire.write(current | (mode << pin));
    Wire.endTransmission();
}

void MCP::pinModeB(int pin, uint8_t mode)    {   
    int8_t current = readDirB();
    if (mode == INPUT) {
        current |= (1 << pin);  // Set bit to 1 for input
    } else {
        current &= ~(1 << pin); // Clear bit to 0 for output
    }
    Wire.beginTransmission(ADDR);
    Wire.write(IODIRB);
    Wire.write(current);
    Wire.endTransmission();
    Serial.println(current);
    delay(500);
}

void MCP::digitalWriteA(uint8_t pin, uint8_t value)  {
    uint8_t current = readPinStateA();
    current &= ~(1 << pin);  // Clear bit first
    current |= (value << pin); // Set bit to value

    Wire.beginTransmission(ADDR);
    Wire.write(GPIOA);
    Wire.write(current);
    Wire.endTransmission();
    Serial.println(current);
    delay(250);
}

void MCP::digitalWriteB(uint8_t pin, uint8_t value)  {
    uint8_t current = readPinStateB();
    current &= ~(1 << pin);  // Clear bit first
    current |= (value << pin); // Set bit to value
   
    Wire.beginTransmission(ADDR);
    Wire.write(GPIOB);
    Wire.write(current);
    Wire.endTransmission();
    Serial.println(current);
    delay(500);
}

uint8_t MCP::digitalReadA(uint8_t pin)    {
    uint8_t current = readPinStateA();
    return (current >> pin) & 1;
}

uint8_t MCP::digitalReadB(uint8_t pin)    {
    uint8_t current = readPinStateB();
    return (current >> pin) & 1;
}

MCP& MCP::Get() {
    static MCP instance;
    return instance;
}

