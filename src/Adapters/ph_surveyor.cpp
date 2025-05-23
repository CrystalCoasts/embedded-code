/*

Library for pH sensor from Atlas Sensors

I doctured the library up from the original code to support an external 12-bit I2C ADC.

This allows the device to run all devices off of I2C, giving making room for external peripherals that take much
more data pins such as cameras.

The original code has not been deleted, however I have commented out the ADC reading functions that correlate to
currently unsupported IO.

-Nathan Chung, 3/11/2025

*/


#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "ph_surveyor.h"
#include "I2Cadc.h"
#include "EEPROM.h"


Surveyor_pH::Surveyor_pH(uint8_t pin){
	this->pin = pin;
    this->EEPROM_offset = (pin) * EEPROM_SIZE_CONST;
    //to lay the calibration parameters out in EEPROM we map their locations to the analog pin numbers
    //we assume a maximum size of EEPROM_SIZE_CONST for every struct we're saving
}

bool Surveyor_pH::begin(){
    #if defined(ESP8266) || defined(ESP32)
        EEPROM.begin(1024);
    #endif 
	if((EEPROM.read(this->EEPROM_offset) == magic_char)
    && (EEPROM.read(this->EEPROM_offset + sizeof(uint8_t)) == SURVEYOR_PH)) {
		EEPROM.get(this->EEPROM_offset,pH);
		return true;
    }
	return false;

  //CONFIG FOR ADC1
  //adc1_config_channel_atten(CHANNEL, ATTENUATION);
}

float Surveyor_pH::read_voltage() {
	float voltage_mV = 0;
  i2cadc.setGain(GAIN_ONE);   //sets voltage reading to +/- 4.096V for I2C ADC
	for (int i = 0; i < volt_avg_len; ++i) {
	#if defined(ESP32)
	//ESP32 has significant nonlinearity in its ADC, we will attempt to compensate 
	//but you're on your own to some extent
	//this compensation is only for the ESP32
	//https://github.com/espressif/arduino-esp32/issues/92
		
    //Arduino code
    //voltage_mV += analogRead(this->pin) / 4095.0 * 3300.0 + 130;

    //READING FROM ADC1
    //voltage_mV += adc1_get_raw(CHANNEL) / 4095.0 * 3300.0 +130;

    //I2C adc code
    voltage_mV += (i2cadc.readADC(this->pin) / 2048) * 4096.0;    //READING from I2C ADC, uses 2048 bits from (0->4.096V)
    //voltage_mV += (i2cadc.readADC(this->pin) / 4096) * 4096.0;    //READING from I2C ADC

	#else
		//voltage_mV += adc1_get_raw(CHANNEL) / 1024.0 * 5000.0;
    voltage_mV += analogRead(this->pin) / 1024.0 * 5000.0;
    #endif 
	}
	voltage_mV /= volt_avg_len;
	return voltage_mV;
}

float Surveyor_pH::read_ph(float voltage_mV) {
  if (voltage_mV > pH.mid_cal) { //high voltage = low ph
    return 7.0 - 3.0 / (this->pH.low_cal - this->pH.mid_cal) * (voltage_mV - this->pH.mid_cal);
  } else {
    return 7.0 - 3.0 / (this->pH.mid_cal - this->pH.high_cal) * (voltage_mV - this->pH.mid_cal);
  }
}

float Surveyor_pH::read_ph() {
  return(read_ph(read_voltage()));
}

void Surveyor_pH::cal_mid(float voltage_mV) {
  this->pH.mid_cal = voltage_mV;
  EEPROM.put(this->EEPROM_offset,pH);
  #if defined(ESP8266) || defined(ESP32)
    EEPROM.commit(); 
  #endif
}

void Surveyor_pH::cal_mid() {
  cal_mid(read_voltage());
}

void Surveyor_pH::cal_low(float voltage_mV) {
  this->pH.low_cal = voltage_mV;
  EEPROM.put(this->EEPROM_offset,pH);
  #if defined(ESP8266) || defined(ESP32)
    EEPROM.commit(); 
  #endif
}

void Surveyor_pH::cal_low() {
  cal_low(read_voltage());
}

void Surveyor_pH::cal_high(float voltage_mV) {
  this->pH.high_cal = voltage_mV;
  EEPROM.put(this->EEPROM_offset,pH);
  #if defined(ESP8266) || defined(ESP32)
    EEPROM.commit(); 
  #endif
}

void Surveyor_pH::cal_high() {
  cal_high(read_voltage());
}

void Surveyor_pH::cal_clear() {
  this->pH.mid_cal = 1500;
  this->pH.low_cal = 2030;
  this->pH.high_cal = 975;
  EEPROM.put(this->EEPROM_offset,pH);
}
