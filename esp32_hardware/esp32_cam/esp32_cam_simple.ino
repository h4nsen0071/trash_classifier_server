/**
 * @file esp32_cam_simple.ino
 * @brief ESP32-CAM đơn giản - Copy từ test file + Serial protocol
 * 
 * ⚠️ QUAN TRỌNG - TRONG ARDUINO IDE:
 * 1. Tools > Board > AI Thinker ESP32-CAM
 * 2. Tools > PSRAM > Enabled  ← BẮT BUỘC!
 * 3. Tools > Partition Scheme > Huge APP (3MB No OTA)
 * 
 * Đổi tên file này thành esp32_cam.ino để sử dụng
 * (hoặc comment/xóa file esp32_cam.ino cũ)
 */

#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ============================================================
// CẤU HÌNH - THAY ĐỔI Ở ĐÂY
// ============================================================

// WiFi
const char* WIFI_SSID = "AMERICANO COFFEE";
const char* WIFI_PASSWORD = "";

// Server
const char* SERVER_HOST = "api.smartbin.live";
const int   SERVER_PORT = 443;
const char* SERVER_PATH = "/classify";

// Serial baud (phải giống Controller)
#define SERIAL_BAUD 115200

// ============================================================
// CAMERA PINS (AI-THINKER ESP32-CAM)
// ============================================================

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ============================================================
// GLOBAL
// ============================================================

bool cameraOK = false;
bool wifiOK = false;
WiFiClientSecure httpsClient;

// ============================================================
// CAMERA INIT (giống hệt test file)
// ============================================================

bool initCamera() {
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
    config.frame_size = FRAMESIZE_UXGA;
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    
    if (psramFound()) {
        config.jpeg_quality = 10;
        config.fb_count = 2;
        config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
    }
    
    esp_err_t err = esp_camera_init(&config);
    
    if (err != ESP_OK) {
        return false;
    }
    
    // Set QVGA sau khi init
    sensor_t* s = esp_camera_sensor_get();
    if (s) {
        s->set_framesize(s, FRAMESIZE_QVGA);
    }
    
    return true;
}

// ============================================================
// WIFI
// ============================================================

bool initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int count = 0;
    while (WiFi.status() != WL_CONNECTED && count < 40) {
        delay(500);
        count++;
    }
    
    return (WiFi.status() == WL_CONNECTED);
}

// ============================================================
// SEND IMAGE & GET RESULT
// ============================================================

int classifyImage(camera_fb_t* fb) {
    // Returns: bin number (1-3), or 0 if error
    
    // Đóng connection cũ trước khi tạo mới (tránh memory leak)
    if (httpsClient.connected()) {
        httpsClient.stop();
    }
    
    // Tạo connection mới mỗi lần
    httpsClient.setInsecure();
    if (!httpsClient.connect(SERVER_HOST, SERVER_PORT, 15000)) {
        return 0;
    }
    
    // Send HTTP request
    httpsClient.printf("POST %s HTTP/1.1\r\n", SERVER_PATH);
    httpsClient.printf("Host: %s\r\n", SERVER_HOST);
    httpsClient.println("Content-Type: image/jpeg");
    httpsClient.printf("Content-Length: %d\r\n", fb->len);
    httpsClient.println("Connection: close");  // Đóng sau mỗi request
    httpsClient.println();
    
    // Send image
    size_t sent = 0;
    while (sent < fb->len) {
        size_t toSend = min((size_t)1024, fb->len - sent);
        size_t written = httpsClient.write(fb->buf + sent, toSend);
        if (written == 0) {
            httpsClient.stop();
            return 0;  // Write failed
        }
        sent += toSend;
    }
    
    // Read response
    String jsonBody = "";
    bool headersEnded = false;
    unsigned long timeout = millis();
    
    while (millis() - timeout < 30000) {
        if (!httpsClient.connected() && !httpsClient.available()) {
            break;
        }
        
        if (httpsClient.available()) {
            String line = httpsClient.readStringUntil('\n');
            
            if (line == "\r" || line.length() == 0) {
                headersEnded = true;
            }
            
            if (headersEnded && line.startsWith("{")) {
                jsonBody = line;
                break;
            }
        }
        delay(1);  // Yield
    }
    
    // Đóng connection
    httpsClient.stop();
    
    if (jsonBody.length() == 0) {
        return 0;
    }
    
    // Parse JSON với error check
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, jsonBody);
    
    if (error) {
        return 0;
    }
    
    // Check success - với null check
    if (!doc.containsKey("success") || !doc["success"].as<bool>()) {
        return 0;
    }
    
    // Check data object exists
    if (!doc.containsKey("data") || !doc["data"].containsKey("bin")) {
        return 0;
    }
    
    // Get bin number
    int binNumber = doc["data"]["bin"].as<int>();
    return binNumber;
}

// ============================================================
// HANDLE CAPTURE COMMAND
// ============================================================

void handleCapture() {
    if (!cameraOK) {
        Serial.println("ERROR:CAMERA_INIT_FAILED");
        return;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("ERROR:WIFI_DISCONNECTED");
        return;
    }
    
    // Capture
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("ERROR:CAPTURE_FAILED");
        return;
    }
    
    // Classify
    int bin = classifyImage(fb);
    
    // Release
    esp_camera_fb_return(fb);
    
    // Send result
    if (bin >= 1 && bin <= 3) {
        Serial.print("BIN:");
        Serial.println(bin);
    } else {
        Serial.println("ERROR:SERVER_ERROR");
    }
}

// ============================================================
// SETUP
// ============================================================

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    
    Serial.begin(SERIAL_BAUD);
    Serial.setDebugOutput(true);
    delay(1000);
    
    // Init camera
    cameraOK = initCamera();
    
    if (!cameraOK) {
        Serial.println("ERROR:CAMERA_INIT_FAILED");
        return;
    }
    
    // Init WiFi
    wifiOK = initWiFi();
    
    if (!wifiOK) {
        Serial.println("ERROR:WIFI_DISCONNECTED");
        return;
    }
    
    // Init HTTPS client
    httpsClient.setInsecure();
    
    // Ready!
    Serial.println("READY");
}

// ============================================================
// LOOP
// ============================================================

void loop() {
    // Maintain WiFi
    if (WiFi.status() != WL_CONNECTED) {
        wifiOK = false;
        WiFi.reconnect();
        delay(5000);
        return;
    }
    wifiOK = true;
    
    // Check commands from Controller
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        
        if (cmd == "CAPTURE") {
            handleCapture();
        } else if (cmd == "PING") {
            Serial.println("PONG");
        } else if (cmd == "STATUS") {
            if (cameraOK && wifiOK) {
                Serial.println("READY");
            } else if (!cameraOK) {
                Serial.println("ERROR:CAMERA_INIT_FAILED");
            } else {
                Serial.println("ERROR:WIFI_DISCONNECTED");
            }
        }
    }
    
    delay(10);
}
