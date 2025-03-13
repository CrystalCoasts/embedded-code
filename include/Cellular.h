/*

Custom made cellular data library for specifically the Smart Sea Wall project. This library uses components from TinyGSM.h, however,
a lot of functions did not end up functioning properly, so I created my own functions using AT commands to send and receive data.

I used this manual to find all the AT commands and their functions: https://cdn.geekfactory.mx/sim7000g/SIM7000%20Series_AT%20Command%20Manual_V1.06.pdf

*/

#ifndef CELLULAR_H
#define CELLUAR_H

#define TINY_GSM_MODEM_SIM7000SSL       //TinyGSM required the model to be specified - Specifically with SSL (https) methods
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#pragma once

#include "TinyGsmClient.h"
#include <ArduinoJson.h>
#include <Arduino.h>
#include <time.h>
#include <HttpClient.h>
#include "esp_heap_caps.h"
#include "io_handler.h"
#include "regex"

//Defining pins and UART
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

class Cellular  {
    public:

        int counter, lastIndex, numberOfPieces = 24;        //used to parse the time of day by counting the iterations and index. Number of pieces refers to settings PDP context
        String pieces[24], input;       //used to parse the time and store it
        bool connected = false;         // bool variable to check if GSM is connected or not
        
        static Cellular& get();
        void begin();

        //Modem power on/off settings
        void modemRestart();
        void modemPowerOff();
        void modemPowerOn();

        //Works with RTC to get current time from cellular NTP
        struct tm getCurrentTime();

        //Server connection functions
        bool serverConnect(const char* server, const char* resource);       //connects to specific server
        void serverDisconnect();        //Disconnects
        bool IsServerConnected();       //Checks if device is connected to domain
        bool isConnected();             //Checks if device is connected to the network, if not tries to reconnect and outputs if it fails or succeeds
        bool isGprsConnected();         //Only checks if device is connected to the network
        bool gprsConnect();             //Attempts to connect to network

        //Get and Post functions
        void sendPostRequest();                         //Testing function to send test data to mock server *working*
        bool sendPostRequest(String jsonPayload);       //Sends json post request to the specified domain *working*
        void sendGetRequest();                          //Get request *working*

        void setHeader(String header, String type);     //sets the json POST header based on the type
        bool setJsonHeader();                           //sets up a premade header for https post *working*

        //Function to send AT commands
        std::string sendData(String command);

    private:

        //make singleton
        Cellular();     //private constructor
        Cellular(const Cellular&) = delete;     //prevent copying
        Cellular& operator=(const Cellular&) = delete;

};

void printHeapStatus(const char* tag);  //Debuging heap *NOT NEEDED ANYMORE*

extern Cellular& sim;   //global variable



#endif