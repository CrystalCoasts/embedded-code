#include "TempSensor.h"

TempSensor& temp = TempSensor::Get();


/**
 * Initializing the sensors instances for temperature and humidity
**/
TempSensor::TempSensor():
    dht(DHTPIN,DHTTYPE),
    oneWire(ONE_WIRE_BUS),
    sensors(&oneWire)
    {}

TempSensor& TempSensor::Get(){
    static TempSensor instance;
    return instance;
}

void TempSensor::begin(){
    dht.begin();
    sensors.begin();
}

bool TempSensor::readHumidity(float* humidity) {
    if (humidity == nullptr) {
        return false; // Invalid pointer
    }

    // Read humidity
    float h = dht.readHumidity();

    // Check if the reading was successful
    if (isnan(h)) {
        return false; // Failed to read humidity
    }

    // Assign the humidity value to the provided pointer
    *humidity = h;
    return true; // Successful read
}

bool TempSensor::readTemperature(TEMP tempScale, float* temperature) {
    if (temperature == nullptr) {
        return false; // Invalid pointer
    }

    sensors.requestTemperatures(); // Request temperature readings

    switch (tempScale) {
        case CELSIUS:
            *temperature = sensors.getTempCByIndex(0); // Read temperature in Celsius
            if (*temperature == DEVICE_DISCONNECTED_C) {
                return false; // Failed to read temperature
            }
            break;

        case FAHRENHEIT:
            *temperature = sensors.getTempFByIndex(0); // Read temperature in Fahrenheit
            if (*temperature == DEVICE_DISCONNECTED_F) {
                return false; // Failed to read temperature
            }
            break;

        default:
            return false; // Unsupported temperature scale
    }

    return true; // Successful read
}