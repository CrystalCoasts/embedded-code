#include "AtlasTemp.h"

AtlasTemp& atlasTempSensor = AtlasTemp::Get(); // Create a global instance of AtlasTemp

AtlasTemp::AtlasTemp() {};

AtlasTemp& AtlasTemp::Get() {
    static AtlasTemp instance; // Guaranteed to be destroyed.
    return instance;           // Instantiated on first use.
}
// Constructor is private to prevent instantiation

void AtlasTemp::begin() {
    mcpGlobal.pinModeA(EN,0);
    wake();
    delay(1000);
    int calibrated = 0, tempComp = 0, salComp = 0;
    char parsedData[32];
    ezoTemp.send_cmd("S,c");
    delay(300);
    sleep();
}

bool AtlasTemp::readTemperature(float* temp) {
    wake();
    delay(1000);
    Wire.beginTransmission(0x66);
    Wire.write("R");
    Wire.endTransmission();
    delay(600);
    ezoTemp.receive_cmd(computerdata, sizeof(computerdata)); //receive readings and stores in ec_data
    delay(500);
    sleep();        //turn off device

    if (computerdata[0] != '\0') {
        *temp = atof(computerdata);        //set DO pointer as data
        return true;
    }
    return false;

}

void AtlasTemp::wake() {
    mcpGlobal.digitalWriteA(EN, HIGH); // Turn on the circuit
}

void AtlasTemp::sleep() {
    mcpGlobal.digitalWriteA(EN, LOW); // Turn off the circuit
}