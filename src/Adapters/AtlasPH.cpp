#include "AtlasPH.h"

AtlasPH& atlasPHSensor = AtlasPH::Get();

AtlasPH::AtlasPH() {};

AtlasPH& AtlasPH::Get()  {
    static AtlasPH instance;
    return instance;
}

void AtlasPH::begin() {
    mcpGlobal.pinModeA(EN,0);
    wake();
    delay(1000);
    int calibrated = 0, tempComp = 0, salComp = 0;
    char parsedData[32];
    ezoPH.send_cmd("T,26"); // Set temperature to 26 degrees Celsius
    delay(300);
    sleep();
}

bool AtlasPH::readPH(float* ph, float temp) {
    wake();
    delay(1000);
    String command = "T," + String(temp,2); // Set temperature compensation
    ezoPH.send_cmd(command.c_str());
    delay(300);
    ezoPH.send_read_cmd();
    delay(900);
    ezoPH.receive_cmd(computerdata, sizeof(computerdata)); //receive readings and stores in ec_data
    ezoPH.send_cmd("Sleep");      //Slepe device
    delay(500);
    sleep();        //turn off device
    if (computerdata[0] != '\0') {
        *ph = atof(computerdata);        //set DO pointer as data
        return true;
    }
    return false;
}

void AtlasPH::wake() {
    mcpGlobal.digitalWriteA(EN, HIGH); // Turn on the circuit
}

void AtlasPH::sleep() {
    mcpGlobal.digitalWriteA(EN, LOW); // Turn off the circuit
}