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