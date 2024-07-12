#include <Arduino.h>
#include <Wire.h>
#include <time.h>
#include <esp_sleep.h>

#define SLAVE_ADDR 0x04
#define MASTER_EN_PIN 25

#define CMD_SEND_TIME 0x01
#define CMD_SET_POWER_ON_TIMER 0x02
#define CMD_POWER_OFF 0x03

volatile time_t nextPowerOn = 0;
volatile bool shouldPowerOff = false;
bool timerIsActive = false;
time_t powerOffTime = 0;

void receiveEvent(int howMany);
void handleCommand(byte command, uint64_t data);
void setTime(uint64_t epoch);

void setup() {
    Serial.begin(115200);
    pinMode(MASTER_EN_PIN, OUTPUT);
    digitalWrite(MASTER_EN_PIN, LOW);
    Serial.println("Master is initially powered off.");

    Wire.begin(SLAVE_ADDR);
    Wire.onReceive(receiveEvent);
    configTime(0, 0, "pool.ntp.org");

    Serial.println("Setup complete. Ready to receive I2C commands.");
    digitalWrite(MASTER_EN_PIN, HIGH); // Power on the master
}

void loop() {
    if (timerIsActive) {
        time_t currentTime = time(NULL);
        if ((currentTime - powerOffTime) >= nextPowerOn) {
            digitalWrite(MASTER_EN_PIN, HIGH);
            Serial.println("Master powered on after scheduled time.");
            timerIsActive = false; // Stop the timer
        }
    }
    delay(1000);
}

void receiveEvent(int howMany) {
    if (howMany < sizeof(uint64_t) + 1) return;
    byte command = Wire.read();
    uint64_t data = 0;
    Wire.readBytes((char*)&data, sizeof(data));
    handleCommand(command, data);
}

void handleCommand(byte command, uint64_t data) {
    switch (command) {
        case CMD_SEND_TIME:
            setTime(data);
            break;
        case CMD_SET_POWER_ON_TIMER:
            nextPowerOn = data;
            Serial.println("Next power on time set: " + String(nextPowerOn));
            break;
        case CMD_POWER_OFF:
            digitalWrite(MASTER_EN_PIN, LOW);
            shouldPowerOff = true;
            timerIsActive = true;
            powerOffTime = time(NULL);
            Serial.println("Master will power off, timer activated.");
            break;
    }
}

void setTime(uint64_t epoch) {
    struct tm timeinfo;
    time_t epochTime = static_cast<time_t>(epoch);
    localtime_r(&epochTime, &timeinfo);
    timeval tv = {epochTime, 0};
    settimeofday(&tv, NULL);
    char buffer[30];
    strftime(buffer, sizeof(buffer), "%c", &timeinfo);
    Serial.print("Updated Local Time: ");
    Serial.println(buffer);
}
