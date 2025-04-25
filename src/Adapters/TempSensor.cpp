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
    wake();
    delay(600);
    dht.begin();
    sensors.begin();
    sleep();
}

bool TempSensor::readTemp(TEMP tempScale, float* temperature)   {
    return true;
}

bool TempSensor::readHumidity(float* humidity) {

    wake();
    delay(600); // Allow time for the sensor to stabilize
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
    sleep();
    return true; // Successful read
}

bool TempSensor::readTemperature(TEMP tempScale, float* temperature) {

    wake();
    delay(600);

    if (temperature == nullptr) {
        return false; // Invalid pointer
    }

    sensors.requestTemperatures(); // Request temperature readings
    Serial.println(sensors.requestTemperatures());
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
            sleep();
            return false; // Unsupported temperature scale
    }
    sleep();

    return true; // Successful read
}

int TempSensor::findDevices()   {
    return 0;
}

void TempSensor::sleep() {
    mcpGlobal.digitalWriteA(EN, LOW); 
} // Turn off the circuit

void TempSensor::wake() {
    mcpGlobal.digitalWriteA(EN, HIGH);
} // Turn on the circuit