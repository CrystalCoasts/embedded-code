#include "Cellular.h"
#include "globals.h"

#define networkTimeout 60000    //1 Minute of network connection waiting
#define checkSignal false

const char apn[]  =  "m2mglobal"; //"iot.1nce.net"     //Set your APN depending on sim used
const char gprsUser[] = "";
const char gprsPass[] = "";

const char server[]   =  "https://128bdb57-9d10-4eb7-b3db-3aa86f885e1c.mock.pstmn.io";      //Domain where you will be sending/receiving data
const char resource[] = "/post";                                                            //The path to where you are sending the data
const int  port       = 443;                                                                //https 443 port, could be port 80 for regular http

TinyGsm modem(mySerial2);
TinyGsmClientSecure client(modem);
HttpClient http(client, server, port);

//Unsused, was used for debugging
void printHeapStatus(const char* tag) {
    Serial.printf("[%s] Free heap: %d, Largest free block: %d\n", 
                  tag, 
                  heap_caps_get_free_size(MALLOC_CAP_8BIT),
                  heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
}

Cellular::Cellular()  {};

Cellular& sim = Cellular::get();

Cellular &Cellular::get()
{
    static Cellular instance;
    return instance;
}

void Cellular::begin()   {
    // Set LED OFF
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    //Turns on SIM chip
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    delay(300);
    digitalWrite(PWR_PIN, LOW);
    delay(1000);

    //GSM Start
    mySerial2.begin(UART_BAUD,SERIAL_8N1, PIN_RX, PIN_TX);

    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    Serial.println("Initializing modem...");
    if (!modem.init()) {
        Serial.println("Failed to initialize modem, attempting to continue without restarting");
    }

    //unlocks sim with the pin
    if ( GSM_PIN && modem.getSimStatus() != 3 ) {
        modem.simUnlock(GSM_PIN);
    }

    //Turns off network functionality, required to set settings like what mode to use (LTE/GSM), (CAT-M, NB-IOT, or Both)
    sendData("AT+CFUN=0");
    delay(200);

    /*
    2 Automatic
    13 GSM only
    38 LTE only
    51 GSM and LTE only
    * * * */
    String res;
    // CHANGE NETWORK MODE, IF NEEDED
    res = modem.setNetworkMode(2);
    if (res != "1") {
        DBG("setNetworkMode  false ");
        return ;
    }
    delay(200);

    /*
        1 CAT-M
        2 NB-Iot
        3 CAT-M and NB-IoT
    * * */
    // CHANGE PREFERRED MODE, IF NEEDED
    res = modem.setPreferredMode(1);    //sets to use CAT-M, a iot based LTE network
    if (res != "1") {
        DBG("setPreferredMode  false ");
        return ;
    }
    delay(200);

    //turns back on the network functionality
    sendData("AT+CFUN=1");
    delay(200);

    /* 
        Connect To PDP (a logical connection between a mobile device and the network that 
        allows for packet-switched data services, like internet access)
    */
    mySerial2.println("AT+CGDCONT?");       //sends at command to check PDP context
    delay(500);

    
    /* 
        bassically what this if statement does is store all the input gathered from the sim chip into a string. and create substrings based  
        on if there are new lines created and stores them in a string array
    */
    if (mySerial2.available()) {
        input = mySerial2.readString();
        for (int i = 0; i < input.length(); i++) {
            if (input.substring(i, i + 1) == "\n") {
                pieces[counter] = input.substring(lastIndex, i);
                lastIndex = i + 1;
                counter++;
            }
            if (i == input.length() - 1) {
                pieces[counter] = input.substring(lastIndex, i);
            }
        }

        // Reset for reuse
        input = "";
        counter = 0;
        lastIndex = 0;


        for ( int y = 0; y < numberOfPieces; y++) {
            for ( int x = 0; x < pieces[y].length(); x++) {
                char c = pieces[y][x];  //gets one byte from buffer
                if (c == ',') {
                    if (input.indexOf(": ") >= 0) {     //if one of the current PDP contexts is >0, store the substring of the entire settings into a string
                        String data = input.substring((input.indexOf(": ") + 1));
                    if ( data.toInt() > 0 && data.toInt() < 25) {       //if that data string is between 0 and 25, set it to the required APN with the follow DNS settings I think
                        modem.sendAT("+CGDCONT=" + String(data.toInt()) + ",\"IP\",\"" + String(apn) + "\",\"0.0.0.0\",0,0,0,0");
                    }
                    input = "";
                    break;
                    }
                // Reset for reuse
                input = "";
                } else {
                input += c;
                }
            }
        }
    } else {
        Serial.println("Failed to get PDP!");
    }

    //Waits for network to connect
    Serial.println("\n\n\nWaiting for network...");
    if (!modem.waitForNetwork(networkTimeout, checkSignal)) {
        delay(10000);
        return;
    }

    //Checks if network is connected
    if (modem.isNetworkConnected()) {
        Serial.println("Network connected");
    }

   //Connection to GPRS
    Serial.println("------- GPRS Test Starting -------");

    Serial.println("Connecting to: " + String(apn));
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {  //attempts to connect to gprs network
        Serial.println("Couldn't connect to GPRS...");  //if it fails, just prints it cant.
    }else{                                              //Else, print the paramets for cellular netowkr
        String ccid = modem.getSimCCID();               //CCID print
        Serial.println("CCID: " + ccid);

        String imei = modem.getIMEI();                  //IMIE print  
        Serial.println("IMEI: " + imei);

        String cop = modem.getOperator();               //Operator print (t-mobile)
        Serial.println("Operator: " + cop);

        IPAddress local = modem.localIP();              //local IP print
        Serial.println("Local IP: " + String(local));

        int csq = modem.getSignalQuality();             // Signal strength (0-31), 99 means unknown or packetloss
        Serial.println("Signal quality: " + String(csq));

        mySerial2.println("AT+CPSI?");     //Get connection type and band
        delay(500);
        if (mySerial2.available()) {
            String r = mySerial2.readString();
            Serial.println(r);
        }
    }
    // sendData("AT+SHSSL?");
}

//Power functions
void Cellular::modemPowerOn(){
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, LOW);
  delay(1000);
  digitalWrite(PWR_PIN, HIGH);
}

void Cellular::modemPowerOff(){
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, LOW);
  delay(1500);
  digitalWrite(PWR_PIN, HIGH);
}


void Cellular::modemRestart(){
  modemPowerOff();
  delay(1000);
  modemPowerOn();
}

bool Cellular::gprsConnect()    {

    //Basically the same code thats in begin() function. Read that to understand whats happening
    if(!sim.isGprsConnected())  {

        digitalWrite(PWR_PIN, HIGH);
        delay(300);
        digitalWrite(PWR_PIN, LOW);
    
        Serial.println("Initializing modem...");
        if (!modem.init()) {
            Serial.println("Failed to restart modem, attempting to continue without restarting");
        }
        
        if ( GSM_PIN && modem.getSimStatus() != 3 ) {
            modem.simUnlock(GSM_PIN);
        }

        sendData("AT+CFUN=0");
        delay(200);

        /*
        2 Automatic
        13 GSM only
        38 LTE only
        51 GSM and LTE only
        * * * */
        String res;
        // CHANGE NETWORK MODE, IF NEEDED
        res = modem.setNetworkMode(2);
        if (res != "1") {
            DBG("setNetworkMode  false ");
            return false;
        }
        delay(200);

        /*
            1 CAT-M
            2 NB-Iot
            3 CAT-M and NB-IoT
        * * */
        // CHANGE PREFERRED MODE, IF NEEDED
        res = modem.setPreferredMode(1);
        if (res != "1") {
            DBG("setPreferredMode  false ");
            return false;
        }
        delay(200);

        sendData("AT+CFUN=1");
        delay(200);

        // CONNECTS TO PDP
        mySerial2.println("AT+CGDCONT?");
        delay(500);
        if (mySerial2.available()) {
            input = mySerial2.readString();
            for (int i = 0; i < input.length(); i++) {
            if (input.substring(i, i + 1) == "\n") {
                pieces[counter] = input.substring(lastIndex, i);
                lastIndex = i + 1;
                counter++;
            }
                if (i == input.length() - 1) {
                pieces[counter] = input.substring(lastIndex, i);
                }
            }
            // Reset for reuse
            input = "";
            counter = 0;
            lastIndex = 0;

            for ( int y = 0; y < numberOfPieces; y++) {
                for ( int x = 0; x < pieces[y].length(); x++) {
                char c = pieces[y][x];  //gets one byte from buffer
                if (c == ',') {
                    if (input.indexOf(": ") >= 0) {
                    String data = input.substring((input.indexOf(": ") + 1));
                    if ( data.toInt() > 0 && data.toInt() < 25) {
                        modem.sendAT("+CGDCONT=" + String(data.toInt()) + ",\"IP\",\"" + String(apn) + "\",\"0.0.0.0\",0,0,0,0");
                    }
                    input = "";
                    break;
                    }
                // Reset for reuse
                input = "";
                } else {
                input += c;
                }
            }
            }
        } else {
            Serial.println("Failed to get PDP!");
            return false;
        }

        Serial.println("\n\n\nWaiting for network...");
        if (!modem.waitForNetwork(networkTimeout, checkSignal)) {
            Serial.println("Waiting for network timed out...");
            return false;
        }

        if (modem.isNetworkConnected()) {
            Serial.println("Network connected");

            String ccid = modem.getSimCCID();
            Serial.println("CCID: " + ccid);

            String imei = modem.getIMEI();
            Serial.println("IMEI: " + imei);

            String cop = modem.getOperator();
            Serial.println("Operator: " + cop);

            IPAddress local = modem.localIP();
            Serial.println("Local IP: " + String(local));

            int csq = modem.getSignalQuality();
            Serial.println("Signal quality: " + String(csq));

            mySerial2.println("AT+CPSI?");     //Get connection type and band
            delay(500);
            if (mySerial2.available()) {
                String r = mySerial2.readString();
                Serial.println(r);
            }
        }else
            Serial.println("Could not connect to network!");
            return false;

        return true;
    }else
        return true;
}

bool Cellular::isConnected()    {
    if(!modem.isGprsConnected())
        if(!sim.gprsConnect())
            return false;
        else
            return true;
    else
        return true;
}

bool Cellular::isGprsConnected()    {
    if(!modem.isNetworkConnected())
        return false;
    else
        return true;
}

std::string Cellular::sendData(String command) {
  // Send the AT command
  mySerial2.println(command);       //sends command to sim
  Serial.println(command);          //prints command on terminal

  // Variables to store the response
  std::string buffer = "";
  unsigned long startTime = millis();       //start time of message sent in milliseconds
  const unsigned long timeout = 60000;      // Timeout in milliseconds

  // Read the response
  while (millis() - startTime < timeout) {      //timeout of 60 seconds
    while (mySerial2.available()) {             //while sim is connected store the string into the buffer
      char c = mySerial2.read();
      buffer += c;

      // Check for termination keywords
      if (buffer.find("OK") != std::string::npos || buffer.find("ERROR") != std::string::npos) {
        break;
      }
    }
    // Break if we already found "OK" or "ERROR"
    if (buffer.find("OK") != std::string::npos || buffer.find("ERROR") != std::string::npos) {
      break;
    }
  }
  // Print the response to the serial monitor
  Serial.println(buffer.c_str());
  return buffer;
}

bool Cellular::serverConnect(const char* server, const char* resource)  {
    Serial.println("Starting connection to " + String(server) + " for json uploads");
    sim.sendData("AT+CCLK?");       //gets time of day
    sim.sendData("AT+CSSLCFG=\"sslversion\",1,3");      //sets SSL version as 3
    sim.sendData("AT+CSSLCFG=\"sni\",1," + String(server));     //sets up config to connnect to domain
    sim.sendData("AT+SHSSL=1,\"\"");        //sets certificate as automatic
    sim.sendData("AT+SHCONF=\"BODYLEN\",1000");         //body length 1000 bytes
    sim.sendData("AT+SHCONF=\"HEADERLEN\",350");        //header length 350 bytes
    sim.sendData("AT+SHCONF=\"URL\"," + String(server));        //domain config set up
    if(sendData("AT+SHCONN").find("ERROR") != std::string::npos)  {     //attempts to connect to domain
        Serial.println("Error found! Could not connnect!");
        sim.connected = false;
        return false;
    }
    Serial.println("Successfully connected to " + String(server));
    sim.connected = true;
    return true;

}

bool Cellular::setJsonHeader()  {
    if(!sim.IsServerConnected()) {
        Serial.println("Server not connected.");
    }else   {
        sim.sendData("AT+SHCHEAD");     //clears head
        sim.sendData("AT+SHAHEAD=\"Content-Type\", \"application/json\"");      //sets json type
        sim.sendData("AT+SHAHEAD=\"User-Agent\",\"curl/7.47.0\"");              //sets curl as user
        sim.sendData("AT+SHAHEAD=\"Cache-control\", \"no-cache\"");             //no cache
        sim.sendData("AT+SHAHEAD=\"Connection\", \"keep-alive\"");              //doesnt let connection die
        sim.sendData("AT+SHAHEAD=\"Accept\", \"*/*\"");                         //Accept any type of data
        //sim.sendData("AT+SHAHEAD=\"Authoriation\", bearer {token} )
        return true;
    }
    
}

bool Cellular::sendPostRequest(String jsonPayload) {
    Serial.println(jsonPayload);
    std::string err;

    jsonLength = jsonPayload.length();  //gets length for sending data

    //Sets escape characters for the 
    jsonPayload.replace("\"", "\\\"");
    jsonPayload = "\"" + jsonPayload + "\""; 

    sim.sendData("AT+SHBOD=" + jsonPayload + "," + String(jsonLength)); // sets the body as the json with the json length
    sim.sendData("AT+SHBOD?");      //checks the body for debugging
    std::string rv = "NULL";
    
    mySerial2.println("AT+SHREQ=" + String(resource) + ",3");       //posts data using the path after domain
    Serial.println("AT+SHREQ=" + String(resource) + ",3");  

    // Variables to store the response
    std::string buffer = "";
    unsigned long startTime = millis();
    const unsigned long timeout = 60000; // Timeout in milliseconds

    // Read the response
    while (millis() - startTime < timeout) {
        while (mySerial2.available()) {
            char c = mySerial2.read();
            buffer += c;

            // Check for termination keywords
            if (buffer.find("SHREQ") != std::string::npos || buffer.find("ERROR") != std::string::npos) {   //checks for SHREQ ending bcuz thats when the response length is given
                for(int i = 0; i<18; i++)    { 
                    c = mySerial2.read();
                    buffer+=c;
                }
                break;
            }
        }
        // Break if we already found "OK" or "ERROR"
        if (buffer.find("SHREQ") != std::string::npos || buffer.find("ERROR") != std::string::npos) {
            break;
        }
    }

    // Print the response to the serial monitor
    Serial.println(buffer.c_str());

    //removes junk characters from reading
    buffer.erase(std::remove_if(buffer.begin(), buffer.end(),
                [](unsigned char c) { return std::isspace(c); }), buffer.end());

    //uses regex to parse the length and error code away from the response
    std::regex re(R"((\d+),(\d+))"); // More flexible regex
    std::smatch match;
    int errorCode = 0;
    int returnBytes = 0;
    
    Serial.print("Buffer Content: ");
    Serial.println(buffer.c_str());
    
    // Print buffer hex values
    Serial.print("Buffer Hex: ");
    for (size_t i = 0; i < buffer.length(); i++) {
        Serial.print("0x");
        Serial.print((uint8_t)buffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    
    // Clean buffer (remove non-printable characters)
    buffer.erase(std::remove_if(buffer.begin(), buffer.end(),
                    [](unsigned char c) { return !std::isprint(c); }), buffer.end());
    
    if (std::regex_search(buffer, match, re)) {
        Serial.println("Regex Matched!");
        Serial.print("Match 1: "); Serial.println(match[1].str().c_str());
        Serial.print("Match 2: "); Serial.println(match[2].str().c_str());
    
        std::string extractedStr = match[2].str();
        Serial.print("Extracted ReturnBytes String: ");
        Serial.println(extractedStr.c_str());
    
        try {
            returnBytes = std::stoi(extractedStr);
            errorCode = std::stoi(match[1].str());
    
            //prints error code and returned bytes
            Serial.print("Error Code: ");
            Serial.println(errorCode);      
            Serial.print("Return Bytes: ");
            Serial.println(returnBytes);
        } catch (const std::exception& e) {
            Serial.print("Conversion Error: ");
            Serial.println(e.what());
        }
    } else {
        Serial.println("Couldn't find matches for the numbers");
    }
    
    
    //print stuff
    char debugBuffer[40];
    sprintf(debugBuffer, "returnBytes = %d", returnBytes);
    Serial.println(debugBuffer);
    
    sprintf(debugBuffer, "AT+SHREAD=0,%d", returnBytes);
    sim.sendData(debugBuffer);

    //Begins reading the response from the post request
    mySerial2.println(debugBuffer);     //sends command to read post request with the recieved bytes of data
    Serial.println(debugBuffer);

    buffer = "";
    startTime = millis();

    // Read the response for a 1 minute timeout
    while (millis() - startTime < timeout) {
        while (mySerial2.available()) {    
            char c = mySerial2.read();      //stores information from SIM in buffer
            buffer += c;

            // Check for termination keywords
            if (buffer.find(returnBytes) != std::string::npos || buffer.find("ERROR") != std::string::npos) {       //if buffer finds the returnBytes value, start saving data from 0 to the received bytes
                for(int i = 0; i<returnBytes; i++)    {
                    c = mySerial2.read();
                    buffer+=c;
                }
                break;
            }
        }
        // Break if we already found "OK" or "ERROR"
        if (buffer.find(returnBytes) != std::string::npos || buffer.find("ERROR") != std::string::npos) {       //break the timeout
            break; 
        }
    }
    Serial.println(buffer.c_str()); //print recieved message
    return true;
}

void Cellular::sendPostRequest() {
  Serial.println("Sending POST request...");

// Header and server connection setup

//   sim.sendData("AT+CSSLCFG=\"sslversion\",1,3");
//   sim.sendData("AT+CSSLCFG=\"sni\",1,\"https://d17e66a7-c349-4d03-9453-cf90701e7aaa.mock.pstmn.io\"");
//   sim.sendData("AT+CCLK?");
//   sim.sendData("AT+SHSSL=1,\"\"");
//   sim.sendData("AT+SHCONF=\"BODYLEN\",1024");
//   sim.sendData("AT+SHCONF=\"HEADERLEN\",350");
//   sim.sendData("AT+SHCONF=\"URL\",\"https://d17e66a7-c349-4d03-9453-cf90701e7aaa.mock.pstmn.io\"");

//   if(sim.sendData("AT+SHCONN").find("ERROR") != std::string::npos)  {
//     Serial.println("Error found! Could not connnect!");
//     return;
//   }

//   sim.sendData("AT+SHSTATE?");
//   sim.sendData("AT+SHCHEAD");
//   sim.sendData("AT+SHAHEAD=\"Content-Type\", \"application/json\"");

//   sim.sendData("AT+SHAHEAD=\"User-Agent\",\"curl/7.47.0\"");
//   sim.sendData("AT+SHAHEAD=\"Cache-control\", \"no-cache\"");
//   sim.sendData("AT+SHAHEAD=\"Connection\", \"keep-alive\"");
//   sim.sendData("AT+SHAHEAD=\"Accept\", \"*/*\"");   
  sim.sendData("AT+SHBOD=\"{\\\"success\\\": \\\"true\\\"}\",19");
  sim.sendData("AT+SHBOD?");  
  mySerial2.println("AT+SHREQ=\"/post\",3");
  Serial.println("AT+SHREQ=\"/post\",3");

  // Variables to store the response
  std::string buffer = "";
  unsigned long startTime = millis();
  const unsigned long timeout = 60000; // Timeout in milliseconds

  // Read the response
  while (millis() - startTime < timeout) {
    while (mySerial2.available()) {
      char c = mySerial2.read();
      buffer += c;

      // Check for termination keywords
      if (buffer.find("SHREQ") != std::string::npos || buffer.find("ERROR") != std::string::npos) {
        int timeNow = millis();
        while(millis() - timeNow < 2000)    {
            c = mySerial2.read();
            buffer += c;
        }
        break;
      }
    }
    // Break if we already found "OK" or "ERROR"
    if (buffer.find("SHREQ") != std::string::npos || buffer.find("ERROR") != std::string::npos) {
      break;
    }
  }
  // Print the response to the serial monitor
  Serial.println(buffer.c_str());

  sim.sendData("AT+SHREAD=0,6");
//   sim.sendData("AT+SHDISC");
}

void Cellular::setHeader(String header, String type)  {    //sets header
    header = "\"" + header + "\"";
    type = "\"" + type + "\"";
    sim.sendData("AT+SHAHEAD=" + header + "," + type);
}


void Cellular::serverDisconnect()   {   //disconnected from domain
    sim.sendData("AT+SHDISC");
}

bool Cellular::IsServerConnected()  {   //checks if device is connected to domain
    if(sim.sendData("AT+SHSTATE?").find('0') != std::string::npos)  {  
        return false;
    }else
        return true;
}


// DONT USE
String prepareJson(String json)  {
    StaticJsonDocument<256> doc;
    String jsonPayload = json;
    serializeJson(doc, jsonPayload);
    jsonPayload.replace("\"", "\\\"");
    jsonPayload = "\"" + jsonPayload + "\""; 
    return jsonPayload;
}