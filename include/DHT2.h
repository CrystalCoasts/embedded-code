/* DHT library


*/


#ifndef DHT2_H
#define DHT2_H

#include <driver/gpio.h>
#include <esp_err.h>
#include <stdint.h>
#include <sys/types.h>
#include <esp_log.h>

#define DHT_OK 0
#define DHT_CHECKSUM_ERROR -1
#define DHT_TIMEOUT_ERROR -2

class DHT2 {

	public:

		DHT2();

		void 	setDHTgpio( gpio_num_t gpio);
		void 	errorHandler(int response);
		int 	readDHT();
		float 	getHumidity();
		float 	getTemperature();

	private:

		gpio_num_t DHTgpio;
		float 	humidity = 0.;
		float 	temperature = 0.;

		int 	getSignalLevel( int usTimeOut, bool state );

};

#endif
