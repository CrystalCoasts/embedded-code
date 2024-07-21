#include "WebSockets.h"
#include "io_handler.h"
#include "globals.h"
#include "rtc_handler.h"

// https://fullstack-web-app-11bd44d807f2.herokuapp.com/


// const char* const WebSocket::WS_SERVER = "fullstack-web-app-11bd44d807f2.herokuapp.com";
// const char* const WebSocket::WS_SERVER = "smart-seawall-server-4c5cb6fd8f61.herokuapp.com";
// const char* const WebSocket::WS_SERVER = "smart-seawall-server-staging-b61a03b529a6.herokuapp.com";
const char* const WebSocket::WS_SERVER = "smart-seawall-7e55bb53fed1.herokuapp.com";

const uint16_t WebSocket::WS_PORT = 80;
const char* const WebSocket::WS_PATH = "/?clientType=esp32";
String WEBSOCKET_TAG = "[WEB_SOCKET] ";

// extern SemaphoreHandle_t sdCardMutex;
// extern SemaphoreHandle_t sensorMutex;
extern TimerHandle_t shutdownTimerHandle;
extern uint16_t batteryLevel;
const uint16_t FULL_BATTERY_CHARGE = 10000; // 10000 mAh


WebSocket& ws = WebSocket::Get(); 

WebSocket& WebSocket::Get() {
    static WebSocket instance;
    return instance;
}

WebSocket::WebSocket() {
    // Private constructor
}

void WebSocket::init() {
    xTaskCreate(webSocketTask, "WebSocketTask", 8192, this, 1, &webSocketTaskHandle);
}

void WebSocket::stop() {
    webSocket.disconnect();
    vTaskDelete(webSocketTaskHandle); // Ensure the task is stopped if running
    Serial.println(WEBSOCKET_TAG + " WebSocket stopped.");
}


void WebSocket::webSocketTask(void * pvParameters) {
    WebSocket *self = static_cast<WebSocket*>(pvParameters);
    self->webSocket.begin(WS_SERVER, WS_PORT, WS_PATH);
    self->webSocket.onEvent(webSocketEvent);
    self->webSocket.setReconnectInterval(5000);

    while (true) {

        // self->send_to_ws("AVAILABLE");
        // delay(10*1000); // for testing one message every 10 seconds
        if(WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi not connected. Skipping ws loop.");
            vTaskDelay(pdMS_TO_TICKS(10000)); // Delay before next execution cycle
            continue;
        }
        self->webSocket.loop();
        vTaskDelay(pdMS_TO_TICKS(10)); // Lower delay to keep WebSocket loop responsive
    }
}

void WebSocket::webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    WebSocket& ws = Get();
    Serial.print(WEBSOCKET_TAG + " Event type: ");
    Serial.println(type);
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println(WEBSOCKET_TAG + " Disconnected");
            break;
        case WStype_CONNECTED:
            Serial.println(WEBSOCKET_TAG + " Connected!");
            break;
        case WStype_TEXT:
            Serial.print(WEBSOCKET_TAG + " Text Message: ");
            for(size_t i = 0; i < length; i++) {
                Serial.print((char) payload[i]);
            }
            Serial.println();
            ws.processPayload(payload, length);
            break;
        case WStype_BIN:
            Serial.print(WEBSOCKET_TAG + " Binary data received, length: ");
            Serial.println(length);
            Serial.print(WEBSOCKET_TAG + " Data: ");
            for(size_t i = 0; i < length; i++) {
                Serial.print(payload[i], HEX); // Print in hexadecimal format
                Serial.print(" ");
            }
            Serial.println();
            ws.processBinaryPayload(payload, length);
            break;
        default:
            Serial.println(WEBSOCKET_TAG + " Unknown event type");
            break;
    }
}

void WebSocket::processBinaryPayload(uint8_t * payload, size_t length) {
    // Convert binary payload to a String
    String payloadString = String((char*)payload, length);
    
    // Print or log the payload for debugging
    Serial.println(WEBSOCKET_TAG + " Processing binary payload");
    Serial.print(WEBSOCKET_TAG + " Binary payload as string: ");
    Serial.println(payloadString);
    
    // Parse the command and data
    String command;
    String data;
    int spaceIndex = payloadString.indexOf(' ');
    
    if (spaceIndex != -1) {
        // Extract command and data
        command = payloadString.substring(0, spaceIndex);
        data = payloadString.substring(spaceIndex + 1);
    } else {
        // No space found, the payload is just the command
        command = payloadString;
        data = ""; // No data provided
    }
    
    // Handle the command with its associated data
    handleCommand(command, data);
}




void WebSocket::send_to_ws(String message)
{
    webSocket.sendTXT(message);
    // delay(300);
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
    String msg;
    // Process the command
    if (command.equals("read")) 
        return handleReadCommand(data);
        
    else if (command.equals("updt")) 
        // Handle "write" command
       return handleUpdateCommand(data);

    
    
    else {
        // Unknown command
        msg = WEBSOCKET_TAG + " Unknown command:" + command;
        Serial.println(msg);
        return (uint8_t)ws_errors::EMPTY_STR;
    }

    return 0;
}

uint8_t WebSocket::handleReadCommand(String cmd_data)
{
    String msg;
    msg = String(WEBSOCKET_TAG + "Handling read command with data: "+ cmd_data);
    Serial.println(msg);

        msg = "[WEB_SOCKET] Handling read command with data: " + cmd_data;
        Serial.println(msg);
        ws.send_to_ws("Read " + cmd_data + " received");

        if (cmd_data.equals("sensors")) {
            Serial.println("[WEB_SOCKET] Reading sensor data");
            SensorData data;
            readSensorData(data);
            String jsonPayload = prepareJsonPayload(data);
            uploadData(jsonPayload);
            ws.send_to_ws("Data uploaded");
        }
    

       else if (cmd_data.equals("bat")) 
       {
            // Read the current battery level and convert it to a percentage
            int batteryPercent = (batteryLevel * 100) / FULL_BATTERY_CHARGE; // Assuming batteryLevel is always up to date
            msg = "Battery: " + String(batteryPercent) + "%";
            ws.send_to_ws(msg);
       }

    return 0;
}



uint8_t WebSocket::handleUpdateCommand(String cmd_data)
{
    String msg;
    msg = String(WEBSOCKET_TAG + "Handling schedule command with data: " + cmd_data);
    Serial.println(msg);

    // Check if the command data is a valid number and not empty
    if (cmd_data.length() == 0 || !cmd_data.toInt()) {
        ws.send_to_ws("Error: Invalid timer value. Command ignored.");
        return 1; // Return an error code or similar
    }

    uint64_t minutes = cmd_data.toInt(); // Convert string to integer
    if (minutes <= 0) {
        ws.send_to_ws("Error: Timer value must be greater than zero. Command ignored.");
        return 1;
    }

    uint64_t microseconds = minutes * MINUTE_US; // Convert minutes to microseconds using macro
    saveTimerSettings(microseconds);

    String ack = "Schedule wakeup in " + cmd_data + " minutes command received and set.";
    ws.send_to_ws(ack);
    ws.send_to_ws("Wake up scheduled");

    return 0; // Success
}
