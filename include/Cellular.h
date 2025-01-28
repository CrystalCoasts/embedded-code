#ifndef CELLULAR_H
#define CELLUAR_H

#define TINY_GSM_MODEM_SIM7000SSL
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#pragma once

#include "TinyGsmClient.h"
#include <ArduinoJson.h>
#include <Arduino.h>
#include <time.h>
#include <HttpClient.h>

#define mySerial2 Serial1
#define UART_BAUD           115200
#define PIN_DTR             25
#define PIN_TX              27
#define PIN_RX              26
#define PWR_PIN             4
#define LED_PIN             12
// set GSM PIN, if any
#define GSM_PIN ""
#define GSM_NL "\n"

 // Your GPRS credentials, if any


class Cellular  {
    public:

        int counter, lastIndex, numberOfPieces = 24;
        String pieces[24], input;

        std::string sendData(String command);
        void modemRestart();
        void modemPowerOff();
        void modemPowerOn();
        struct tm getCurrentTime();
        void sendPostRequest();
        void sendPostRequest(String jsonPayload, const char* server, const char* resource);
        void sendGetRequest();
        bool isConnected();
        bool isGprsConnected();
        bool gprsConnect();
        void setHeader(String header, String type); 
        // String prepareJson();
        // uint8_t printLocalTime();
        // void updateSystemTime(const struct tm& newTime);

        static Cellular& get();
        void begin();



    private:
        Cellular();
        Cellular(const Cellular&) = delete;
        Cellular& operator=(const Cellular&) = delete;

};

extern Cellular& sim;



#endif