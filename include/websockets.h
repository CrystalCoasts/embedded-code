#ifndef WEBSOCKETS_H
#define WEBSOCKETS_H

#include <WebSocketsClient.h>
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

//commands
enum class ws_cmd {
    READ,
    TEST,
    UPDT, //to update values like schedule next read
};
enum class ws_errors{
    EMPTY_STR = 1
};

class WebSocket {
public:
    void init();
    void stop();
    void begin();  // Now used to check if the WebSocket needs reconnection or other periodic checks
    static WebSocket& Get();
    void send_to_ws(String message);

private:
    WebSocket();
    WebSocket(const WebSocket&) = delete;
    WebSocket& operator=(const WebSocket&) = delete;

    static void webSocketTask(void * pvParameters); // Task for managing WebSocket connection
    static void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
    void handleEvents();

    WebSocketsClient webSocket;
    TaskHandle_t webSocketTaskHandle = NULL;

    static const char* const WS_SERVER;
    static const uint16_t WS_PORT;
    static const char* const WS_PATH;

    uint8_t processPayload(uint8_t * payload, size_t length);
    uint8_t handleCommand(String command, String data);

    // Individual command handlers
    uint8_t handleReadCommand(String data);
    uint8_t handleUpdateCommand(String data);
};

extern WebSocket& ws;  // Global WebSocket instance

#endif // WEBSOCKETS_H
