#ifndef CAMERA_H
#define CAMERA_H

#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include "SPI.h"
#include <EEPROM.h>            // read and write from flash memory

#define CAMERA_MODEL_ESP_EYE
#define EEPROM_SIZE 1

#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     -1
#define SIOD_GPIO_NUM     21
#define SIOC_GPIO_NUM     22
#define Y9_GPIO_NUM       23
#define Y8_GPIO_NUM       19
#define Y7_GPIO_NUM       18
#define Y6_GPIO_NUM       5
#define Y5_GPIO_NUM       39
#define Y4_GPIO_NUM       36
#define Y3_GPIO_NUM       26
#define Y2_GPIO_NUM       27
#define VSYNC_GPIO_NUM    34
#define HREF_GPIO_NUM     35
#define PCLK_GPIO_NUM     25

#define CLK 14
#define CMD 15
#define DATA 2
#define SD_CD_PIN 13

class Camera {
public:
    camera_config_t config;
    camera_fb_t* fb = NULL; // Frame buffer pointer
    static Camera& get(); // Singleton instance
    void begin();
    bool takePicture(const String& path);
    bool savePictureToSDCard(const String& path);
    void turnOffFlash();
    void turnOnFlash();
    void setFlashPin(int pin);
    int getPictureNumber() const;
    void incrementPictureNumber();
    void resetPictureNumber();
    void setPictureNumber(int number);

private:

    int pictureNumber = 0; // Initialize picture number to 0
    const char* TAG = "Camera";
    const char* SD_TAG = "[SD_CARD]";

            //make singleton
    Camera();     //private constructor
    Camera(const Camera&) = delete;     //prevent copying
    Camera& operator=(const Camera&) = delete;
};

    extern Camera& cam; // Declare the camera instance
    // extern camera_config_t config;
    // extern camera_fb_t* fb = NULL; // Frame buffer pointer


#endif // CAMERA_H