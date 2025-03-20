#include "DOSensor.h"
#include <iostream>


DOSensor& DO = DOSensor::get();

DOSensor::DOSensor()    {};

DOSensor &DOSensor::get()
{
    static DOSensor instance;
    return instance;
}

void DOSensor::begin()
{   
    mcpGlobal.pinModeA(EN_O,0);
    wake();
    int calibrated = 0, tempComp = 0, salComp = 0;
    char parsedData[32];
    delay(500);

    // if (parseValue(ec_data, parsedData, "?Cal")) {
    //     Serial.println("Calibration profile detected.");
    //     calibrated = (atoi(parsedData) != 0);
    // }

    // Sends a calibration command to calibrate DO to atmospheric oxygen levels
    // ec2.send_cmd("Cal");
    // delay(1300);
    // Serial.println("DO Calibration Complete")

    ec2.send_cmd("T,?");
    delay(500);
    ec2.receive_cmd(ec_data, sizeof(ec_data));

    if (parseValue(ec_data, parsedData, "?T")) {        //sets temperature for reading accurately
        Serial.println("Correct Temp compensation.");
        tempComp = (atoi(parsedData) == TEMP_COMP);
    }
    if (!tempComp) {
        String command = "T," + String(TEMP_COMP);
        ec2.send_cmd(command.c_str());
        delay(500);
    }

    ec2.send_cmd("S,?");
    delay(500);
    ec2.receive_cmd(ec_data, sizeof(ec_data));

    if (parseValue(ec_data, parsedData, "?S")) {        //sets salinity for reading accurately
        Serial.println("Correct Conversion Factor.");
        salComp = (atoi(parsedData) == SAL_COMP);
    }
    if (!salComp) {
        String command = "S," + String(SAL_COMP);
        ec2.send_cmd(command.c_str());
        delay(500);
    }
    sleep();
}

bool DOSensor::readDO(float* DO, float salinity, float temp) {

    // digitalWrite(EN_O, HIGH);
    // delay(300);
    wake();
    delay(500);

    //must send salinity and temp for proper reading
    String command = "S," + String(salinity) + ",ppt";
    ec2.send_cmd(command.c_str());      //send command with current salinity value
    delay(300);
    command = "T," + String(temp);
    ec2.send_cmd(command.c_str());      //send command with current temperature value
    delay(300);

    ec2.send_read_cmd();
    delay(600);
    ec2.receive_cmd(ec_data, sizeof(ec_data));  //read DO data
    ec2.send_cmd("Sleep");      //Slepe device
    delay(500);
    sleep();        //turn off device
    
    if (ec_data[0] != '\0') {
        *DO = atof(ec_data);        //set DO pointer as data
        return true;
    }
    return false;

}

bool DOSensor::parseValue(const char* rawBuff, char* parsedBuff, const char* key) { //parses values for Conversion Factor
        const char* found = strstr(rawBuff, key);
    if (found) {
        strcpy(parsedBuff, found + strlen(key) + 1);
        return true;
    }
    return false;
}

void DOSensor::sleep()  {   //sleeps device by setting EN low
    mcpGlobal.digitalWriteA(EN_O, LOW);
}

void DOSensor::wake()   {   //wakes device by setting EN high
    mcpGlobal.digitalWriteA(EN_O, HIGH);
}