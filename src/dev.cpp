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

// define the number of bytes you want to access
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

int pictureNumber = 0;

void setup() {
  Serial.begin(115200);

//camera init
  SPI.begin(CLK,DATA,CMD,SD_CD_PIN);
  cam.begin();

  Serial.println("Starting SD Card");
  if(!SD.begin(SD_CD_PIN, SPI)){
    Serial.println("SD Card Mount Failed");
    return;
  }
  cam.takePicture("/");

  delay(2000);
  Serial.println("Going to sleep now");
  delay(2000);
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void loop() {
}