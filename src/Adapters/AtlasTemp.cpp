#include "AtlasTemp.h"

AtlasTemp& AtlasTemp::get() {
    static AtlasTemp instance; // Guaranteed to be destroyed.
    return instance;           // Instantiated on first use.
}
// Constructor is private to prevent instantiation

AtlasTemp::AtlasTemp() {};

void AtlasTemp::begin() {
    mcpGlobal.pinModeA(EN_O,0);
    wake();
    int calibrated = 0, tempComp = 0, salComp = 0;
    char parsedData[32];
    ec.send_cmd("S,c");
    delay(300);
    sleep();
}

bool AtlasTemp::readTemp(float* temp) {
    wake();
    ec.send_read_cmd();
    delay(600);
    ec.receive_cmd(ec_data, sizeof(ec_data)); //receive readings and stores in ec_data
    ec.send_cmd("Sleep");      //Slepe device
    delay(500);
    sleep();        //turn off device
}

void AtlasTemp::wake() {
    mcpGlobal.digitalWriteA(EN_O, HIGH); // Turn on the circuit
}

void AtlasTemp::sleep() {
    mcpGlobal.digitalWriteA(EN_O, LOW); // Turn off the circuit
}