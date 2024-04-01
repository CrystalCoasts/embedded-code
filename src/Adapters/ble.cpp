#include "ble.h"

Ble& Ble::Get() {
    static Ble instance; // Guaranteed to be created only once
    return instance;
}

Ble::Ble() {
    // Constructor remains empty; use begin() for initialization
}

void Ble::begin() {
    BLEDevice::init("ESP32 Sensor Node");
    pServer = BLEDevice::createServer();
    setupServiceAndCharacteristics();
    pServer->getAdvertising()->start();
    Serial.println("BLE ready");
}

void Ble::setupServiceAndCharacteristics() {
    BLEService* pService = pServer->createService(serviceUUID);
    pCharacteristic = pService->createCharacteristic(
                         characteristicUUID,
                         BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
                     );
    // Configure additional characteristic settings if needed
    pService->start();
}

void Ble::sendSensorData(const std::string& data) {
    if (pCharacteristic != nullptr) {
        pCharacteristic->setValue(data);
        pCharacteristic->notify(); // Notify connected client
    }
}
