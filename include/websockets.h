#ifndef WEBSOCKETS_H
#define WEBSOCKETS_H

#include <WebSocketsClient.h>
#include <Arduino.h>

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
    void init();  // Initialize WebSocket connection
    void begin();  //call loop
    static WebSocket& Get();
    static void webSocketEvent(WStype_t type, uint8_t* payload, size_t length);

    void send_to_ws(String message);

private:
    WebSocket();  // Private constructor
    WebSocket(const WebSocket&) = delete;  // No copy constructor
    WebSocket& operator=(const WebSocket&) = delete;  // No copy assignment

    WebSocketsClient webSocket;

    static const char* const WS_SERVER;
    static const uint16_t WS_PORT;
    static const char* const WS_PATH;

    uint8_t processPayload(uint8_t * payload, size_t length);
    uint8_t handleCommand(String command, String data);


    //individual command handlers
    uint8_t handleReadCommand(String data);
    uint8_t handleUpdateCommand(String data);
};


//might mess up status tracking
extern WebSocket& ws;  // Global WebSocket instance



#endif // WEBSOCKETS_H
