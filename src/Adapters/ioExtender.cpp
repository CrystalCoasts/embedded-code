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
    Wire.beginTransmission(ADDR);       //begin i2c transmisions
    Wire.write(GPIOA);                  //access gpioA data bits
    Wire.endTransmission();             //end writing

    Wire.requestFrom(ADDR, (uint8_t) 1);        //request byte from GPIOA bus
    if(Wire.available())    {
        uint8_t out = Wire.read();              //read 
        Serial.println(out);
        return out;                             //return bus
    }
    return 0;
}

uint8_t MCP::readPinStateB()    {
    Wire.beginTransmission(ADDR);           //begin i2c transmission
    Wire.write(GPIOB);                      //access gpioB
    Wire.endTransmission();                 //end writing transmision

    Wire.requestFrom(ADDR, (uint8_t) 1);    //send read request
    if(Wire.available())    {
        uint8_t out = Wire.read();          //sets read into variable
        Serial.println(out);
        Wire.endTransmission();             //end read request
        delay(200);
        return out;                         //return GPIOB bus
    }
    return 0;
}

uint8_t MCP::readDirA()  {
    Wire.beginTransmission(ADDR);           //begin i2c transmission
    Wire.write(IODIRA);                     //access IO directin A (out/input)
    Wire.endTransmission();                 //end write request
    Wire.requestFrom(ADDR, (uint8_t) 1);    //begin read request
    if(Wire.available())  {
        uint8_t out = Wire.read();          //read register
        Serial.println(out);
        return out;    }                    //return register
    return 0;
}

uint8_t MCP::readDirB()  {
    Wire.beginTransmission(ADDR);           //begin i2c transmission
    Wire.write(IODIRB);                     //access io direction B (out/input)
    Wire.endTransmission();
    Wire.requestFrom(ADDR, (uint8_t) 1);    //request read
    if(Wire.available())  {
        uint8_t out = Wire.read();          //read io direction register
        Serial.println(out);
        return out;    }                    //return regist value
    return 0;
}

void MCP::pinModeA(int pin, uint8_t mode)    {   
    int8_t current = readDirA();        //get current io directions for bus A
    if (mode == INPUT) {
        current |= (1 << pin);          // Set bit to 1 for input
    } else {
        current &= ~(1 << pin);         // Clear bit to 0 for output
    }
    Wire.beginTransmission(ADDR);       //begin i2c
    Wire.write(IODIRA);                 // access io direction A  
    Wire.write(current | (mode << pin));    //set pin to new pinmode (keep the rest the same)
    Wire.endTransmission();
}

void MCP::pinModeB(int pin, uint8_t mode)    {   
    int8_t current = readDirB();        //get current io direction for bus B
    if (mode == INPUT) {
        current |= (1 << pin);  // Set bit to 1 for input
    } else {
        current &= ~(1 << pin); // Clear bit to 0 for output
    }
    Wire.beginTransmission(ADDR);       //begin i2c 
    Wire.write(IODIRB);                 //access io direction B register
    Wire.write(current);                //overwrite pin direction with new direction
    Wire.endTransmission();             //end i2c
    Serial.println(current);
    delay(500);
}

void MCP::digitalWriteA(uint8_t pin, uint8_t value)  {
    uint8_t current = readPinStateA();          //find current values in gpio register
    current &= ~(1 << pin);                     // Clear bit first
    current |= (value << pin);                  // Set bit to value

    Wire.beginTransmission(ADDR);               //begin i2c
    Wire.write(GPIOA);                          //access gpioA
    Wire.write(current);                        //write new digital write register
    Wire.endTransmission();                     //end i2c
    Serial.println(current);
    delay(250);
}

void MCP::digitalWriteB(uint8_t pin, uint8_t value)  {
    uint8_t current = readPinStateB();          //read current gpio register
    current &= ~(1 << pin);                     // Clear pin bit first
    current |= (value << pin);                  // Set pin bit to value
   
    Wire.beginTransmission(ADDR);               //begin i2c
    Wire.write(GPIOB);                          //access gpio B bus
    Wire.write(current);                        //write new pin value
    Wire.endTransmission();                     //end i2c
    Serial.println(current);
    delay(500);
}

uint8_t MCP::digitalReadA(uint8_t pin)    {
    uint8_t current = readPinStateA();          //read all pin values (1 bytes)
    return (current >> pin) & 1;
}

uint8_t MCP::digitalReadB(uint8_t pin)    {
    uint8_t current = readPinStateB();          //read all pin values (1 bytes)
    return (current >> pin) & 1;
}

MCP& MCP::Get() {
    static MCP instance;
    return instance;
}

