//UART testing for 

#include "Arduino.h"
#include "Wire.h"
//#include "esp_camera.h"
#include "HardwareSerial.h"

#define TX 1
#define RX 3

HardwareSerial esp32(1);

void setup()    {
    Serial.begin(115200);
    esp32.begin(115200, SERIAL_8N1, RX, TX);
}

void loop() {
    for(int i = 0; i <10; i++)
        esp32.write(2);
    delay(500);
}