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
    int calibrated = 0, tempComp = 0, salComp = 0;
    char parsedData[32];
    delay(500);

     if (parseValue(ec_data, parsedData, "?Cal")) {
        Serial.println("Calibration profile detected.");
        calibrated = (atoi(parsedData) != 0);
    }

    ec2.send_cmd("T,?");
    delay(500);
    ec2.receive_cmd(ec_data, sizeof(ec_data));

    if (parseValue(ec_data, parsedData, "?T")) {
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

    if (parseValue(ec_data, parsedData, "?S")) {
        Serial.println("Correct Conversion Factor.");
        salComp = (atoi(parsedData) == SAL_COMP);
    }
    if (!salComp) {
        String command = "S," + String(SAL_COMP);
        ec2.send_cmd(command.c_str());
        delay(500);
    }

}

bool DOSensor::readDO(float* DO, float salinity, float temp) {

    //must send salinity and temp for proper reading
    String command = "S," + String(salinity) + ",ppt";
    ec2.send_cmd(command.c_str());
    delay(300);
    command = "T," + String(temp);
    ec2.send_cmd(command.c_str());
    delay(300);

    ec2.send_read_cmd();
    delay(600);
    ec2.receive_cmd(ec_data, sizeof(ec_data));
    // Serial.print("RAW salinity Read: ");
    // Serial.println(ec_data);

    if (ec_data[0] != '\0') {
        *DO = atof(ec_data);
        return true;
    }
    return false;

}

bool DOSensor::parseValue(const char* rawBuff, char* parsedBuff, const char* key) {
        const char* found = strstr(rawBuff, key);
    if (found) {
        strcpy(parsedBuff, found + strlen(key) + 1);
        return true;
    }
    return false;
}