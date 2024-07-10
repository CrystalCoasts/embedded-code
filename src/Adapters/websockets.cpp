#include "WebSockets.h"
#include "websockets.h"

// Initialization of static constants

const char* const WebSocket::WS_SERVER = "smart-seawall-server-staging-b61a03b529a6.herokuapp.com";
const uint16_t WebSocket::WS_PORT = 80;
const char* const WebSocket::WS_PATH = "/?clientType=esp32";

WebSocket& ws = WebSocket::Get();  // Create the global WebSocket instance

WebSocket::WebSocket() {
    // Private constructor
}

void WebSocket::init() {
    
    webSocket.begin(WS_SERVER, WS_PORT, WS_PATH);
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
}

void WebSocket::begin() {
    // Serial.println("WebSocket loop start");
    webSocket.loop();
    // Serial.println("WebSocket loop end");
}


WebSocket& WebSocket::Get() {
    static WebSocket instance;
    return instance;
}

void WebSocket::webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("[WebSocket] Disconnected!");
            break;
        case WStype_CONNECTED:
            Serial.println("[WebSocket] Connected to server");
            ws.send_to_ws("Hello from ESP32!");  // Send a message upon connection
            break;
        case WStype_TEXT:
            
            ws.processPayload(payload,length );
            break;
    }
}

void WebSocket::send_to_ws(String message)
{
    webSocket.sendTXT(message);
}

uint8_t WebSocket::processPayload(uint8_t * payload, size_t length) {
    // Convert uint8_t * to String
    String payloadString = String((char *)payload);

    // Find the position of the first space
    int spaceIndex = payloadString.indexOf(' ');

    if (spaceIndex != -1) {
        // Extract the command and data
        String command = payloadString.substring(0, spaceIndex);
        String data = payloadString.substring(spaceIndex + 1);

        // Handle the command based on its value
        handleCommand(command, data);
    } else {
        // No space found, handle as necessary (assuming payload is just the command)
        handleCommand(payloadString, "");
    }

    return 0;
}

uint8_t WebSocket::handleCommand(String command, String data) {
    // Process the command
    if (command.equals("read")) 
        return handleReadCommand(data);
        
    else if (command.equals("updt")) 
        // Handle "write" command
       return handleUpdateCommand(data);

    
    
    else {
        // Unknown command
        Serial.println("Unknown command: " + command);
        return (uint8_t)ws_errors::EMPTY_STR;
    }

    return 0;
}

uint8_t WebSocket::handleReadCommand(String data)
{
    Serial.println("Handling read command with data: " + data);
    ws.send_to_ws("Read received"); 
    return 0;
}

uint8_t WebSocket::handleUpdateCommand(String data)
{
    Serial.println("Handling updt command with data: " + data);
    ws.send_to_ws("Update next wakeup received"); 
    return 0;
}
