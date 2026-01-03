/**
 * @file test_esp32_cam.ino
 * @brief Test ESP32-CAM - FIX Stack Overflow
 * 
 * ⚠️ QUAN TRỌNG - TRONG ARDUINO IDE:
 * 1. Tools > Board > AI Thinker ESP32-CAM
 * 2. Tools > PSRAM > Enabled  ← BẮT BUỘC!
 * 3. Tools > Partition Scheme > Huge APP (3MB No OTA)
 */

#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ============================================================
// CẤU HÌNH - THAY ĐỔI Ở ĐÂY
// ============================================================

// WiFi
const char* WIFI_SSID = "AMERICANO COFFEE";
const char* WIFI_PASSWORD = "";

// Server - thay đổi theo môi trường
const char* SERVER_HOST = "ideal-potato-v6rxqj9qrxq7cv9g-5000.app.github.dev";  // Domain hoặc IP
const int   SERVER_PORT = 443;                   // 443 cho HTTPS (Codespace), 5000 cho local
const char* SERVER_PATH = "/classify";           // Endpoint path
const bool  USE_HTTPS = true;                    // true = HTTPS, false = HTTP

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
WiFiClientSecure client;  // Global để reuse connection

// ============================================================
// CAMERA INIT
// ============================================================

bool initCamera() {
    Serial.println("[CAM] Init...");
    
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
    
    // Copy từ CameraWebServer - hoạt động tốt
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_UXGA;
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    
    // Nếu có PSRAM - tăng chất lượng
    if (psramFound()) {
        Serial.println("[CAM] PSRAM found!");
        config.jpeg_quality = 10;
        config.fb_count = 2;
        config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
        Serial.println("[CAM] No PSRAM - using SVGA");
        config.frame_size = FRAMESIZE_SVGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
    }
    
    esp_err_t err = esp_camera_init(&config);
    
    if (err != ESP_OK) {
        Serial.printf("[CAM] FAILED! Error: 0x%x\n", err);
        return false;
    }
    
    // Giữ mặc định - không chỉnh gì thêm
    sensor_t* s = esp_camera_sensor_get();
    if (s) {
        // QVGA (320x240) - phù hợp vì model resize về 224x224
        // Không cần ảnh lớn hơn, chỉ tốn băng thông
        s->set_framesize(s, FRAMESIZE_QVGA);
    }
    
    Serial.println("[CAM] OK!");
    return true;
}

// ============================================================
// WIFI
// ============================================================

bool initWiFi() {
    Serial.printf("[WIFI] Connecting to %s\n", WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int count = 0;
    while (WiFi.status() != WL_CONNECTED && count < 30) {
        delay(500);
        Serial.print(".");
        count++;
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("[WIFI] OK! IP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    }
    
    Serial.println("[WIFI] FAILED!");
    return false;
}

// ============================================================
// CAPTURE & SEND
// ============================================================

void captureAndSend() {
    Serial.println("\n--- CAPTURE ---");
    Serial.printf("[MEM] Free heap: %d\n", ESP.getFreeHeap());
    
    if (!cameraOK) {
        Serial.println("[ERR] Camera not ready");
        return;
    }
    
    // Check memory first
    if (ESP.getFreeHeap() < 20000) {
        Serial.println("[ERR] Low memory! Skipping...");
        return;
    }
    
    // Check WiFi first (before capture)
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[ERR] WiFi disconnected, reconnecting...");
        WiFi.reconnect();
        delay(3000);
        return;
    }
    
    // Capture frame
    camera_fb_t* fb = esp_camera_fb_get();
    
    if (!fb) {
        Serial.println("[ERR] Capture failed");
        return;
    }
    
    Serial.printf("[CAM] Size: %d bytes, %dx%d\n", fb->len, fb->width, fb->height);
    
    // Send to server với retry
    Serial.printf("[HTTP] Connecting to %s...\n", SERVER_HOST);
    
    int retries = 3;
    bool connected = false;
    
    while (retries > 0 && !connected) {
        if (!client.connected()) {
            if (USE_HTTPS) {
                client.setInsecure();  // Skip certificate for HTTPS
            }
            connected = client.connect(SERVER_HOST, SERVER_PORT, 10000);  // 10s timeout
        } else {
            connected = true;
        }
        if (!connected) {
            Serial.printf("[HTTP] Retry... (%d left)\n", --retries);
            delay(500);
        }
    }
    
    if (!connected) {
        Serial.println("[HTTP] Connection failed after retries!");
        esp_camera_fb_return(fb);
        return;
    }
    
    // Send HTTP request
    client.printf("POST %s HTTP/1.1\r\n", SERVER_PATH);
    client.printf("Host: %s\r\n", SERVER_HOST);
    client.println("Content-Type: image/jpeg");
    client.printf("Content-Length: %d\r\n", fb->len);
    client.println("Connection: keep-alive");
    client.println();
    
    // Send image data in chunks
    size_t sent = 0;
    size_t chunkSize = 1024;
    while (sent < fb->len) {
        size_t toSend = min(chunkSize, fb->len - sent);
        client.write(fb->buf + sent, toSend);
        sent += toSend;
    }
    
    // Release frame immediately
    esp_camera_fb_return(fb);
    
    Serial.printf("[HTTP] Sent %d bytes\n", sent);
    
    // Wait for response
    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 30000) {
        if (client.available()) {
            String line = client.readStringUntil('\n');
            Serial.println(line);
            if (line.startsWith("{")) {
                // JSON response
                Serial.println("[HTTP] Response received!");
                break;
            }
        }
    }
    
    client.stop();
    Serial.printf("[MEM] Free heap after: %d\n", ESP.getFreeHeap());
}

// ============================================================
// SETUP
// ============================================================

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    delay(1000);
    
    Serial.println("\n\n============================");
    Serial.println("   ESP32-CAM TEST v5");
    Serial.println("   Default settings");
    Serial.println("============================\n");
    
    Serial.printf("[SYS] Free heap: %d\n", ESP.getFreeHeap());
    Serial.printf("[SYS] PSRAM: %s\n", psramFound() ? "YES" : "NO");
    if (psramFound()) {
        Serial.printf("[SYS] PSRAM size: %d\n", ESP.getPsramSize());
    }
    Serial.println();
    
    // Init camera
    cameraOK = initCamera();
    delay(500);
    
    // Init WiFi
    wifiOK = initWiFi();
    
    // Init global client
    client.setInsecure();
    
    Serial.println("\n============================");
    Serial.printf("Camera: %s\n", cameraOK ? "OK" : "FAILED");
    Serial.printf("WiFi:   %s\n", wifiOK ? "OK" : "FAILED");
    Serial.println("============================\n");
    
    if (!psramFound()) {
        Serial.println("!!! PSRAM NOT FOUND !!!");
        Serial.println("Go to Arduino IDE:");
        Serial.println("1. Tools > Board > AI Thinker ESP32-CAM");
        Serial.println("2. Tools > PSRAM > Enabled");
        Serial.println("3. Reupload");
        Serial.println();
    }
}

// ============================================================
// LOOP
// ============================================================

void loop() {
    if (cameraOK && wifiOK) {
        captureAndSend();
    } else {
        Serial.println("[LOOP] Waiting... (Camera or WiFi not ready)");
    }
    
    delay(10000);  // 10 giây
}
