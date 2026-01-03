/**
 * @file camera_handler.cpp
 * @brief Camera Operations Implementation
 */

#include "camera_handler.h"
#include "config.h"
#include "camera_pins.h"
#include "base64.h"

// ============================================================
// PRIVATE VARIABLES
// ============================================================

static bool cameraInitialized = false;
static size_t lastImageSize = 0;

// ============================================================
// DEBUG LOGGING
// ============================================================

#if defined(DEBUG_ENABLED) && defined(DEBUG_CAMERA)
    #define LOG_CAM(msg) Serial.print("[CAMERA] "); Serial.println(msg)
    #define LOG_CAM_VAL(msg, val) Serial.print("[CAMERA] "); Serial.print(msg); Serial.println(val)
#else
    #define LOG_CAM(msg)
    #define LOG_CAM_VAL(msg, val)
#endif

// ============================================================
// PUBLIC FUNCTIONS
// ============================================================

bool cameraHandler_init() {
    LOG_CAM("Initializing...");
    
    // Camera configuration
    camera_config_t config;
    
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    
    // QUAN TRỌNG: Init với UXGA trước, set framesize sau
    // Giống như CameraWebServer example - tránh lỗi cam_task stack overflow
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    
    // Check PSRAM
    if (psramFound()) {
        LOG_CAM("PSRAM found - using high quality settings");
        config.jpeg_quality = 10;
        config.fb_count = 2;
        config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
        LOG_CAM("No PSRAM - limiting frame size");
        config.frame_size = FRAMESIZE_SVGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
    }
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    
    if (err != ESP_OK) {
        LOG_CAM_VAL("Init failed with error: ", err);
        cameraInitialized = false;
        return false;
    }
    
    // Apply camera settings và set frame size thực tế
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor != NULL) {
        // Set frame size QVGA (320x240) - phù hợp với model 224x224
        sensor->set_framesize(sensor, CAMERA_FRAME_SIZE);
        
        sensor->set_brightness(sensor, CAMERA_BRIGHTNESS);
        sensor->set_contrast(sensor, CAMERA_CONTRAST);
        sensor->set_saturation(sensor, CAMERA_SATURATION);
        
        // Flip image if mounted upside down
        // sensor->set_vflip(sensor, 1);
        // sensor->set_hmirror(sensor, 1);
    }
    
    // Setup flash LED
    pinMode(FLASH_GPIO_NUM, OUTPUT);
    digitalWrite(FLASH_GPIO_NUM, LOW);
    
    cameraInitialized = true;
    LOG_CAM("Initialized successfully");
    
    // Print camera info
    #if defined(DEBUG_ENABLED) && defined(DEBUG_CAMERA)
        const char* frameSizeName;
        switch (CAMERA_FRAME_SIZE) {
            case FRAMESIZE_QVGA:    frameSizeName = "QVGA (320x240)"; break;
            case FRAMESIZE_VGA:     frameSizeName = "VGA (640x480)"; break;
            case FRAMESIZE_SVGA:    frameSizeName = "SVGA (800x600)"; break;
            case FRAMESIZE_XGA:     frameSizeName = "XGA (1024x768)"; break;
            default:                frameSizeName = "Unknown"; break;
        }
        LOG_CAM_VAL("Frame size: ", frameSizeName);
        LOG_CAM_VAL("JPEG quality: ", CAMERA_JPEG_QUALITY);
    #endif
    
    return true;
}

camera_fb_t* cameraHandler_capture() {
    if (!cameraInitialized) {
        LOG_CAM("ERROR: Camera not initialized");
        return NULL;
    }
    
    #if USE_FLASH_LED
        cameraHandler_flashOn();
        delay(100); // Let the flash stabilize
    #endif
    
    LOG_CAM("Capturing image...");
    unsigned long startTime = millis();
    
    camera_fb_t* fb = esp_camera_fb_get();
    
    #if USE_FLASH_LED
        cameraHandler_flashOff();
    #endif
    
    if (!fb) {
        LOG_CAM("ERROR: Capture failed");
        lastImageSize = 0;
        return NULL;
    }
    
    lastImageSize = fb->len;
    
    LOG_CAM_VAL("Capture time (ms): ", millis() - startTime);
    LOG_CAM_VAL("Image size (bytes): ", fb->len);
    LOG_CAM_VAL("Image width: ", fb->width);
    LOG_CAM_VAL("Image height: ", fb->height);
    
    return fb;
}

void cameraHandler_releaseFrame(camera_fb_t* fb) {
    if (fb) {
        esp_camera_fb_return(fb);
        LOG_CAM("Frame released");
    }
}

bool cameraHandler_captureToBase64(String& base64Output) {
    camera_fb_t* fb = cameraHandler_capture();
    
    if (!fb) {
        return false;
    }
    
    LOG_CAM("Encoding to base64...");
    unsigned long startTime = millis();
    
    // Encode to base64
    base64Output = base64::encode(fb->buf, fb->len);
    
    LOG_CAM_VAL("Encode time (ms): ", millis() - startTime);
    LOG_CAM_VAL("Base64 size: ", base64Output.length());
    
    // Release frame
    cameraHandler_releaseFrame(fb);
    
    return true;
}

void cameraHandler_flashOn() {
    digitalWrite(FLASH_GPIO_NUM, HIGH);
    LOG_CAM("Flash ON");
}

void cameraHandler_flashOff() {
    digitalWrite(FLASH_GPIO_NUM, LOW);
    LOG_CAM("Flash OFF");
}

size_t cameraHandler_getLastImageSize() {
    return lastImageSize;
}
