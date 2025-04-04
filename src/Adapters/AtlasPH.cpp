#include "AtlasPH.h"

void AtlasPH::begin() {
    mcpGlobal.pinModeA(EN_O,0);
    wake();
    int calibrated = 0, tempComp = 0, salComp = 0;
    char parsedData[32];
    ec.send_cmd("T,26"); // Set temperature to 26 degrees Celsius
    delay(300);
    sleep();
}

bool AtlasPH::readTemp(float* ph, int temp) {
    wake();
    delay(300);
    ec.send_cmd("T," + temp); // Get temperature compensation
    delay(300);
    ec.send_read_cmd();
    delay(600);
    ec.receive_cmd(ec_data, sizeof(ec_data)); //receive readings and stores in ec_data
    ec.send_cmd("Sleep");      //Slepe device
    delay(500);
    sleep();        //turn off device
}

void AtlasPH::wake() {
    mcpGlobal.digitalWriteA(EN_O, HIGH); // Turn on the circuit
}

void AtlasPH::sleep() {
    mcpGlobal.digitalWriteA(EN_O, LOW); // Turn off the circuit
}