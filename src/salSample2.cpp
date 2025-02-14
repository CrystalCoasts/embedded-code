#include <Arduino.h>
#include "pHSensor.h"
#include "I2Cadc.h"


uint8_t user_bytes_received = 0;                
const uint8_t bufferlen = 32;                   
char user_data[bufferlen];                     

void parse_cmd(char* string) {                   
  strupr(string);                                
  if (strcmp(string, "CAL,7") == 0) {       
    phGloabl.cal_mid();                                
    Serial.println("MID CALIBRATED");
  }
  else if (strcmp(string, "CAL,4") == 0) {            
    phGloabl.cal_low();                                
    Serial.println("LOW CALIBRATED");
  }
  else if (strcmp(string, "CAL,10") == 0) {      
    phGloabl.cal_high();                               
    Serial.println("HIGH CALIBRATED");
  }
  else if (strcmp(string, "CAL,CLEAR") == 0) { 
    phGloabl.cal_clear();                              
    Serial.println("CALIBRATION CLEARED");
  }
}

void setup()    {
    Serial.begin(115200);   
    i2cadc.begin();                         
    delay(200);
    Serial.println(F("Use commands \"CAL,7\", \"CAL,4\", and \"CAL,10\" to calibrate the circuit to those respective values"));
    Serial.println(F("Use command \"CAL,CLEAR\" to clear the calibration"));
    if (phGloabl.begin()) {                                     
      Serial.println("Loaded EEPROM");
    }
}

void loop(){
    if (Serial.available() > 0) {                                                      
        user_bytes_received = Serial.readBytesUntil(13, user_data, sizeof(user_data));   
      }
    
      if (user_bytes_received) {                                                      
        parse_cmd(user_data);                                                          
        user_bytes_received = 0;                                                        
        memset(user_data, 0, sizeof(user_data));                                         
      }
      float value;
      phGloabl.readpH(&value);
      Serial.println(value);                                                      
      delay(1000);
}


// // #include <Arduino.h>
// // #include "SalinitySensor.h"

// // void setup(){
// //     sal.begin();
// //     sal.EnableDisableSingleReading(SAL,1);
// // }

// // void loop(){
// //     float salinity;
// //     sal.readSalinity(&salinity);
// //     Serial.println(salinity);
// // }


// #include <Arduino.h>
// #include "SalinitySensor.h"

// void setup() {
//     Wire.begin();
//     Serial.begin(115200);
//     sal.begin();
//     sal.EnableDisableSingleReading(SAL, 1);
//     Serial.println("Initialization complete");
// }

// void loop() {
//     float salinity;
//     if (sal.readSalinity(&salinity)) {
//         Serial.print("Salinity: ");
//         Serial.println(salinity);
//     } else {
//         Serial.println("Failed to read salinity.");
//     }
//     delay(2000);
// }
// #include <Arduino.h>
// #include "SalinitySensor.h"

// void setup(){
//     Wire.begin();
//     Serial.begin(115200);
//     sal.begin();
//     sal.EnableDisableSingleReading(SAL,1);
// }

// void loop(){
//     float salinity;
//     sal.readSalinity(&salinity);
//     Serial.println(salinity);
// }