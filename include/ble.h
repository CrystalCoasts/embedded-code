#ifndef BLE_H
#define BLE_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Arduino.h>


class Ble {
public:
    static Ble& Get();

    void begin(); // Setup BLE
    void sendSensorData(const std::string& data); // Generic method to send data

private:
    Ble(); // Private constructor
    void setupServiceAndCharacteristics();

    BLEServer* pServer = nullptr;
    BLECharacteristic* pCharacteristic = nullptr;

    // ble.h
    static constexpr const char* const serviceUUID = "your-service-uuid";
    static constexpr const char* const characteristicUUID = "your-characteristic-uuid";


    // Singleton pattern
    Ble(const Ble&) = delete;
    Ble& operator=(const Ble&) = delete;
};

#endif // BLE_H
