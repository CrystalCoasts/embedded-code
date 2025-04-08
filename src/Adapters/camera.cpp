#include "camera.h"

//camera_config_t cam_config;

Camera& cam = Camera::get();

Camera::Camera() {};

Camera &Camera::get()
{
    static Camera instance;
    return instance;
}

void Camera::begin() {
    EEPROM.begin(EEPROM_SIZE);

    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

    cam.config.ledc_channel = LEDC_CHANNEL_0;
    cam.config.ledc_timer = LEDC_TIMER_0;
    cam.config.pin_d0 = Y2_GPIO_NUM;
    cam.config.pin_d1 = Y3_GPIO_NUM;
    cam.config.pin_d2 = Y4_GPIO_NUM;
    cam.config.pin_d3 = Y5_GPIO_NUM;
    cam.config.pin_d4 = Y6_GPIO_NUM;
    cam.config.pin_d5 = Y7_GPIO_NUM;
    cam.config.pin_d6 = Y8_GPIO_NUM;
    cam.config.pin_d7 = Y9_GPIO_NUM;
    cam.config.pin_xclk = XCLK_GPIO_NUM;
    cam.config.pin_pclk = PCLK_GPIO_NUM;
    cam.config.pin_vsync = VSYNC_GPIO_NUM;
    cam.config.pin_href = HREF_GPIO_NUM;
    cam.config.pin_sscb_sda = SIOD_GPIO_NUM;
    cam.config.pin_sscb_scl = SIOC_GPIO_NUM;
    cam.config.pin_pwdn = PWDN_GPIO_NUM; // Power down is not used
    cam.config.pin_reset = RESET_GPIO_NUM; // Software reset will be performed
    cam.config.xclk_freq_hz = 20000000; // XCLK frequency
    cam.config.pixel_format = PIXFORMAT_JPEG; 

    if(psramFound()){
        cam.config.frame_size = FRAMESIZE_UXGA; // 1600x1200
        cam.config.jpeg_quality = 12; // JPEG quality (0-63, lower is better quality)
        cam.config.fb_count = 3; // Number of frame buffers
    } else {
        cam.config.frame_size = FRAMESIZE_SVGA; // 800x600
        cam.config.jpeg_quality = 12; // JPEG quality (0-63, lower is better quality)
        cam.config.fb_count = 1; // Number of frame buffers
    }

    // Initialize the camera
    esp_err_t err = esp_camera_init(&cam.config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
    Serial.println("Camera initialized successfully.");
}

bool Camera::takePicture(const String& path) {
    cam.fb = NULL; // Reset the frame buffer pointer
    cam.fb = esp_camera_fb_get(); // Get the frame buffer from the camera
    if (!fb) {
        Serial.println("Camera capture failed");
        return false;
    }
    if(savePictureToSDCard(path))   {// Save the picture to SD card
        esp_camera_fb_return(cam.fb); // Return the frame buffer to the camera driver
        return true; // Return success
    }else{
        esp_camera_fb_return(cam.fb); // Return the frame buffer to the camera driver
        return false; // Return failure
    }

}

bool Camera::savePictureToSDCard(const String& path) {
    if (cam.fb) {
        EEPROM.begin(EEPROM_SIZE);
        pictureNumber = EEPROM.read(0) + 1;
        fs::FS &fs = SD;
        File file = fs.open("/picture" + String(pictureNumber) + ".jpg", FILE_WRITE); // Open the file for writing
        if (!file) {
            Serial.println("Failed to open file for writing");
            return false;
        }else{
            file.write(cam.fb->buf, cam.fb->len); // Write the frame buffer to the file
            EEPROM.write(0, pictureNumber); // Write the new picture number to EEPROM
            EEPROM.commit(); // Commit the changes to EEPROM
            Serial.println("Picture saved to SD card successfully.");
            return true;
        }
        file.close(); // Close the file
    } else {
        Serial.println("No frame buffer available to save picture.");
        return false;
    }
}

void Camera::setPictureNumber(int number) {
    pictureNumber = number; // Set the picture number
    EEPROM.write(0, pictureNumber); // Write the picture number to EEPROM
    EEPROM.commit(); // Commit the changes to EEPROM
}

void Camera::incrementPictureNumber() {
    pictureNumber++; // Increment the picture number
    EEPROM.write(0, pictureNumber); // Write the new picture number to EEPROM
    EEPROM.commit(); // Commit the changes to EEPROM
}

void Camera::resetPictureNumber() {
    pictureNumber = 0; // Reset the picture number to 0
    EEPROM.write(0, pictureNumber); // Write the new picture number to EEPROM
    EEPROM.commit(); // Commit the changes to EEPROM
}