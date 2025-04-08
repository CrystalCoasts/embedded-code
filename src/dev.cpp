// /**
//  * This example takes a picture every 5s and print its size on serial monitor.
//  */

// // =============================== SETUP ======================================

// // 1. Board setup (Uncomment):
// // #define BOARD_WROVER_KIT
// // #define BOARD_ESP32CAM_AITHINKER

// /**
//  * 2. Kconfig setup
//  *
//  * If you have a Kconfig file, copy the content from
//  *  https://github.com/espressif/esp32-camera/blob/master/Kconfig into it.
//  * In case you haven't, copy and paste this Kconfig file inside the src directory.
//  * This Kconfig file has definitions that allows more control over the camera and
//  * how it will be initialized.
//  */

// /**
//  * 3. Enable PSRAM on sdkconfig:
//  *
//  * CONFIG_ESP32_SPIRAM_SUPPORT=y
//  *
//  * More info on
//  * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/kconfig.html#config-esp32-spiram-support
//  */

// // ================================ CODE ======================================

// #include "Arduino.h"
// #include <esp_log.h>
// #include <esp_system.h>
// #include <nvs_flash.h>
// #include <sys/param.h>
// #include <string.h>

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// // support IDF 5.x
// #ifndef portTICK_RATE_MS
// #define portTICK_RATE_MS portTICK_PERIOD_MS
// #endif

// #include "esp_camera.h"

// #define BOARD_WROVER_KIT

// // WROVER-KIT PIN Map
// #ifdef BOARD_WROVER_KIT

// #define CAM_PIN_PWDN -1  //power down is not used
// #define CAM_PIN_RESET -1 //software reset will be performed
// #define CAM_PIN_XCLK 25
// #define CAM_PIN_SIOD 21
// #define CAM_PIN_SIOC 22

// #define CAM_PIN_D7 3
// #define CAM_PIN_D6 39
// #define CAM_PIN_D5 12
// #define CAM_PIN_D4 23
// #define CAM_PIN_D3 19
// #define CAM_PIN_D2 18
// #define CAM_PIN_D1 5
// #define CAM_PIN_D0 36
// #define CAM_PIN_VSYNC 35
// #define CAM_PIN_HREF 34
// #define CAM_PIN_PCLK 1

// #endif

// // ESP32Cam (AiThinker) PIN Map
// // #ifdef BOARD_ESP32CAM_AITHINKER

// //     #define CAM_PIN_PWDN 32
// //     #define CAM_PIN_RESET -1 //software reset will be performed
// //     #define CAM_PIN_XCLK 0
// //     #define CAM_PIN_SIOD 26
// //     #define CAM_PIN_SIOC 27

// //     #define CAM_PIN_D7 35
// //     #define CAM_PIN_D6 34
// //     #define CAM_PIN_D5 39
// //     #define CAM_PIN_D4 36
// //     #define CAM_PIN_D3 21
// //     #define CAM_PIN_D2 19
// //     #define CAM_PIN_D1 18
// //     #define CAM_PIN_D0 5
// //     #define CAM_PIN_VSYNC 25
// //     #define CAM_PIN_HREF 23
// //     #define CAM_PIN_PCLK 22

// // #endif

// static const char *TAG = "example:take_picture";

// #if ESP_CAMERA_SUPPORTED
// static camera_config_t camera_config = {
//     .pin_pwdn = CAM_PIN_PWDN,
//     .pin_reset = CAM_PIN_RESET,
//     .pin_xclk = CAM_PIN_XCLK,
//     .pin_sccb_sda = CAM_PIN_SIOD,
//     .pin_sccb_scl = CAM_PIN_SIOC,

//     .pin_d7 = CAM_PIN_D7,
//     .pin_d6 = CAM_PIN_D6,
//     .pin_d5 = CAM_PIN_D5,
//     .pin_d4 = CAM_PIN_D4,
//     .pin_d3 = CAM_PIN_D3,
//     .pin_d2 = CAM_PIN_D2,
//     .pin_d1 = CAM_PIN_D1,
//     .pin_d0 = CAM_PIN_D0,
//     .pin_vsync = CAM_PIN_VSYNC,
//     .pin_href = CAM_PIN_HREF,
//     .pin_pclk = CAM_PIN_PCLK,

//     //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
//     .xclk_freq_hz = 20000000,
//     .ledc_timer = LEDC_TIMER_0,
//     .ledc_channel = LEDC_CHANNEL_0,

//     .pixel_format = PIXFORMAT_RGB565, //YUV422,GRAYSCALE,RGB565,JPEG
//     .frame_size = FRAMESIZE_QVGA,    //QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

//     .jpeg_quality = 12, //0-63, for OV series camera sensors, lower number means higher quality
//     .fb_count = 1,       //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
//     .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
// };

// static esp_err_t init_camera(void)
// {
//     //initialize the camera
//     esp_err_t err = esp_camera_init(&camera_config);
//     if (err != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Camera Init Failed");
//         return err;
//     }

//     return ESP_OK;
// }
// #endif

// void setup()
// {
//     Serial.begin(115200);
// #if ESP_CAMERA_SUPPORTED
//     if(ESP_OK != init_camera()) {
//         return;
//     }

//     while (1)
//     {
//         //ESP_LOGI(TAG, "Taking picture...");
//         Serial.println("Taking photo");
//         camera_fb_t *pic = esp_camera_fb_get();

//         // use pic->buf to access the image
//         // ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", pic->len);
//         Serial.printf("Picture taken! Size was: %d bytes", pic->len);
//         esp_camera_fb_return(pic);

//         vTaskDelay(5000 / portTICK_RATE_MS);
//     }
// #else
//     ESP_LOGE(TAG, "Camera support is not available for this chip");
//     return;
// #endif
// }

// void loop() {

// }

/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-cam-take-photo-save-microsd-card
  
  IMPORTANT!!! 
   - Select Board "AI Thinker ESP32-CAM"
   - GPIO 0 must be connected to GND to upload a sketch
   - After connecting GPIO 0 to GND, press the ESP32-CAM on-board RESET button to put your board in flashing mode
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/
#include "camera.h"
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

//const int CLK = 5;
//const int CMD = 4;
//const int DATA = 6;
//const int SD_CD_PIN = 46;

// define the number of bytes you want to access
#define EEPROM_SIZE 1

// Pin definition for CAMERA_MODEL_AI_THINKER
//#define PWDN_GPIO_NUM     -1
//#define RESET_GPIO_NUM    -1
//#define XCLK_GPIO_NUM     34
//#define SIOD_GPIO_NUM     15
//#define SIOC_GPIO_NUM     16
//#define Y9_GPIO_NUM       14
//#define Y8_GPIO_NUM       13
//#define Y7_GPIO_NUM       12
//#define Y6_GPIO_NUM       11
//#define Y5_GPIO_NUM       10
//#define Y4_GPIO_NUM       9
//#define Y3_GPIO_NUM       8
//#define Y2_GPIO_NUM       7
//#define VSYNC_GPIO_NUM    36
//#define HREF_GPIO_NUM     35
//#define PCLK_GPIO_NUM     37

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

int pictureNumber = 0;

void setup() {
 
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  //Serial.println();
  
//camera init
  SPI.begin(CLK,DATA,CMD,SD_CD_PIN);
  cam.begin();


  // camera_config_t config;
  // config.ledc_channel = LEDC_CHANNEL_0;
  // config.ledc_timer = LEDC_TIMER_0;
  // config.pin_d0 = Y2_GPIO_NUM;
//   config.pin_d1 = Y3_GPIO_NUM;
//   config.pin_d2 = Y4_GPIO_NUM;
//   config.pin_d3 = Y5_GPIO_NUM;
//   config.pin_d4 = Y6_GPIO_NUM;
//   config.pin_d5 = Y7_GPIO_NUM;
//   config.pin_d6 = Y8_GPIO_NUM;
//   config.pin_d7 = Y9_GPIO_NUM;
//   config.pin_xclk = XCLK_GPIO_NUM;
//   config.pin_pclk = PCLK_GPIO_NUM;
//   config.pin_vsync = VSYNC_GPIO_NUM;
//   config.pin_href = HREF_GPIO_NUM;
//   config.pin_sccb_sda = SIOD_GPIO_NUM;
//   config.pin_sccb_scl = SIOC_GPIO_NUM;
//   config.pin_pwdn = PWDN_GPIO_NUM;
//   config.pin_reset = RESET_GPIO_NUM;
//   config.xclk_freq_hz = 20000000;
//   config.pixel_format = PIXFORMAT_JPEG; 
  
//   if(psramFound()){
//     config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
//     config.jpeg_quality = 10;
//     config.fb_count = 2;
//   } else {
//     config.frame_size = FRAMESIZE_SVGA;
//     config.jpeg_quality = 12;
//     config.fb_count = 1;
//   }
  
//   // Init Camera
//   esp_err_t err = esp_camera_init(&config);
//   if (err != ESP_OK) {
//     Serial.printf("Camera init failed with error 0x%x", err);
//     return;
//   }

// //    if(!SD_MMC.setPins(CLK, CMD, DATA)){
// //    //if(! SD_MMC.setPins(clk, cmd, d0, d1, d2, d3)){
// //        Serial.println("Pin change failed!");
// //        return;
// //    }
// //  
  Serial.println("Starting SD Card");
  if(!SD.begin(SD_CD_PIN, SPI)){
    Serial.println("SD Card Mount Failed");
    return;
  }
  
// //  uint8_t cardType = SD_MMC.cardType();
// //  if(cardType == CARD_NONE){
// //    Serial.println("No SD Card attached");
// //    return;
// //  }
  cam.takePicture("/");
  
//   // Take Picture with Camera

//   camera_fb_t* fb = NULL;
//   fb = esp_camera_fb_get();  
//   if(!fb) {
//     Serial.println("Camera capture failed");
//     return;
//   }
//   Serial.println("capture successful!");
//   // initialize EEPROM with predefined size
//   EEPROM.begin(EEPROM_SIZE);
//   pictureNumber = EEPROM.read(0) + 1;

//   // Path where new picture will be saved in SD Card
//   String path = "/picture" + String(pictureNumber) +".jpg";

//   fs::FS &fs = SD; 
//   Serial.printf("Picture file name: %s\n", path.c_str());
  
//   File file = fs.open(path.c_str(), FILE_WRITE);
//   if(!file){
//     Serial.println("Failed to open file in writing mode");
//   } 
//   else {
//     file.write(fb->buf, fb->len); // payload (image), payload length
//     Serial.printf("Saved file to path: %s\n", path.c_str());
//     EEPROM.write(0, pictureNumber);
//     EEPROM.commit();
//   }
//   file.close();
//   esp_camera_fb_return(fb); 

//   Serial.printf("Picture number: %d\n", pictureNumber);
  //cam.takePicture("/");




  
  // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
//  pinMode(4, OUTPUT);
//  digitalWrite(4, LOW);
//  rtc_gpio_hold_en(GPIO_NUM_4);
//  
  delay(2000);
  Serial.println("Going to sleep now");
  delay(2000);
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void loop() {
  
}