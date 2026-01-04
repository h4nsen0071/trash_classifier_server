/*
 * ESP32-CAM với ESP-NOW + HTTPS (SSL/TLS)
 * ==========================================
 *
 * Phiên bản hỗ trợ server production với SSL certificate
 *
 * Thay đổi từ HTTP sang HTTPS:
 * - Dùng WiFiClientSecure thay cho WiFiClient
 * - Port 443 thay cho 5000
 * - Có thể verify certificate hoặc skip (setInsecure)
 */

#include "esp_camera.h"
#include <WiFi.h>
#include <esp_now.h>
#include <esp_mac.h>
#include <WiFiClientSecure.h> // ← THAY ĐỔI: dùng Secure client
#include <ArduinoJson.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ============================================================
// CẤU HÌNH - THAY ĐỔI Ở ĐÂY
// ============================================================

// WiFi
const char *WIFI_SSID = "ZTE 2.4G";
const char *WIFI_PASSWORD = "";

// ============================================================
// SERVER CONFIG - CHỌN 1 TRONG 2 CHẾ ĐỘ
// ============================================================

// --- CHẾ ĐỘ 1: Local testing (HTTP) ---
// #define USE_HTTPS false
// const char *SERVER_HOST = "192.168.2.5";
// const int SERVER_PORT = 5000;

// --- CHẾ ĐỘ 2: Production (HTTPS) ---
#define USE_HTTPS true
const char *SERVER_HOST = "your-domain.com"; // ← THAY domain của bạn
const int SERVER_PORT = 443;

const char *SERVER_PATH = "/classify";

// SSL/TLS được Cloudflare xử lý, chỉ cần setInsecure()

// ============================================================
// ESP-NOW: MAC Address của ESP32 Controller
// ============================================================
uint8_t controllerMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // ← THAY ĐỔI!

// ESP-NOW Message structure
typedef struct struct_message
{
    char command[32];
    char response[64];
} struct_message;

// ============================================================
// CAMERA PINS (AI-THINKER ESP32-CAM)
// ============================================================

#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// ============================================================
// GLOBAL
// ============================================================

bool cameraOK = false;
bool wifiOK = false;
bool espnowOK = false;
bool captureRequested = false;

// ← THAY ĐỔI: Dùng WiFiClientSecure cho HTTPS
WiFiClientSecure httpsClient;

struct_message incomingMsg;
struct_message outgoingMsg;
esp_now_peer_info_t peerInfo;

// ============================================================
// FUNCTION DECLARATIONS
// ============================================================

bool initCamera();
bool initWiFi();
bool initESPNow();
void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status);
void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len);
int classifyImage(camera_fb_t *fb);
void sendResponse(const char *response);
void processCaptureRequest();

// ============================================================
// SETUP
// ============================================================

void setup()
{
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n========================================");
    Serial.println("   ESP32-CAM + ESP-NOW + HTTPS");
    Serial.println("========================================\n");

    // 1. Init Camera
    Serial.println("[1/3] Initializing camera...");
    cameraOK = initCamera();
    if (!cameraOK)
    {
        Serial.println("[FATAL] Camera init failed!");
    }
    else
    {
        Serial.println("[OK] Camera ready!\n");
    }

    // 2. Init ESP-NOW
    Serial.println("[2/3] Initializing ESP-NOW...");
    espnowOK = initESPNow();
    if (!espnowOK)
    {
        Serial.println("[FATAL] ESP-NOW init failed!");
    }
    else
    {
        Serial.println("[OK] ESP-NOW ready!\n");
    }

    // 3. Connect WiFi (vẫn cần cho HTTP request)
    Serial.println("[3/3] Connecting to WiFi...");
    wifiOK = initWiFi();
    if (!wifiOK)
    {
        Serial.println("[FATAL] WiFi connection failed!");
    }
    else
    {
        Serial.println("[OK] WiFi connected!\n");

// SSL/TLS do Cloudflare xử lý
#if USE_HTTPS
        httpsClient.setInsecure();     // Cloudflare lo SSL
        httpsClient.setTimeout(15000); // 15s timeout
        Serial.println("[SSL] HTTPS mode enabled (Cloudflare)");
#endif
    }

    // Summary
    Serial.println("========================================");
    Serial.println("           INITIALIZATION RESULT        ");
    Serial.println("========================================");
    Serial.printf("Camera:   %s\n", cameraOK ? "OK" : "FAILED");
    Serial.printf("ESP-NOW:  %s\n", espnowOK ? "OK" : "FAILED");
    Serial.printf("WiFi:     %s\n", wifiOK ? "OK" : "FAILED");
    Serial.printf("Mode:     %s\n", USE_HTTPS ? "HTTPS (SSL)" : "HTTP");
    Serial.printf("Server:   %s:%d\n", SERVER_HOST, SERVER_PORT);
    Serial.println("========================================\n");

    if (cameraOK && espnowOK && wifiOK)
    {
        Serial.println("[READY] System ready! Waiting for commands...\n");
        sendResponse("READY");
    }
    else
    {
        Serial.println("[ERROR] System not fully initialized!\n");
    }
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // Xử lý capture request (được set từ ESP-NOW callback)
    if (captureRequested)
    {
        captureRequested = false;
        processCaptureRequest();
    }

    delay(10);
}

// ============================================================
// PROCESS CAPTURE REQUEST (gọi từ loop, KHÔNG phải từ callback)
// ============================================================

void processCaptureRequest()
{
    unsigned long startTotal = millis();
    
    if (!cameraOK)
    {
        Serial.println("[ERROR] Camera not initialized!");
        sendResponse("ERROR:CAMERA");
        return;
    }

    if (!wifiOK)
    {
        Serial.println("[ERROR] WiFi not connected!");
        sendResponse("ERROR:WIFI");
        return;
    }

    Serial.println("\n[CAM] === CAPTURE START ===");
    
    // ============================================
    // PRE-CAPTURE: Warm-up camera for stability
    // ============================================
    // Discard first 2 frames to let auto-exposure/white balance stabilize
    for (int i = 0; i < 2; i++)
    {
        camera_fb_t *fb_temp = esp_camera_fb_get();
        if (fb_temp)
        {
            esp_camera_fb_return(fb_temp);
            delay(100); // Wait for camera to adjust
        }
    }
    
    Serial.println("[CAM] Pre-capture warm-up complete");
    
    // Now capture the actual image
    unsigned long t1 = millis();
    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb)
    {
        Serial.println("[ERROR] Camera capture failed!");
        sendResponse("ERROR:CAPTURE");
        return;
    }

    Serial.printf("[CAM] Image captured: %d bytes [%dms]\n", fb->len, millis() - t1);
    
    // ============================================
    // IMAGE QUALITY VALIDATION
    // ============================================
    // Check if image is too small (likely corrupt)
    if (fb->len < 5000)
    {
        Serial.printf("[ERROR] Image too small (%d bytes), likely corrupt\n", fb->len);
        esp_camera_fb_return(fb);
        sendResponse("ERROR:CAPTURE");
        return;
    }
    
    // Check if image is unusually large (potential issue)
    if (fb->len > 100000)
    {
        Serial.printf("[WARN] Image very large (%d bytes), may have quality issues\n", fb->len);
    }
    
    Serial.println("[CAM] Image quality check passed");

    // Send to server for classification
    unsigned long t2 = millis();
    int binNumber = classifyImage(fb);
    Serial.printf("[TIMING] Classification: %dms\n", millis() - t2);

    // Return frame buffer
    esp_camera_fb_return(fb);

    // Send result via ESP-NOW
    unsigned long t3 = millis();
    if (binNumber > 0)
    {
        char response[32];
        sprintf(response, "BIN:%d", binNumber);
        Serial.printf("[RESULT] Classification: Bin %d\n", binNumber);
        sendResponse(response);
    }
    else
    {
        Serial.println("[ERROR] Classification failed!");
        sendResponse("ERROR:CLASSIFY");
    }
    Serial.printf("[TIMING] ESP-NOW send: %dms\n", millis() - t3);
    Serial.printf("[TIMING] === TOTAL: %dms ===\n\n", millis() - startTotal);
}

// ============================================================
// INIT CAMERA
// ============================================================

bool initCamera()
{
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

    if (psramFound())
    {
        config.jpeg_quality = 10;
        config.fb_count = 2;
        config.grab_mode = CAMERA_GRAB_LATEST;
    }
    else
    {
        config.frame_size = FRAMESIZE_SVGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.println("[ERROR] Camera init failed!");
        return false;
    }

    // Configure sensor for better image quality
    sensor_t *s = esp_camera_sensor_get();
    if (s)
    {
        // ============================================
        // IMPROVED IMAGE QUALITY SETTINGS
        // ============================================
        
        // Set VGA (640x480) - Better than QVGA for classification
        s->set_framesize(s, FRAMESIZE_VGA);
        
        // Auto settings for consistent quality
        s->set_whitebal(s, 1);      // Auto white balance ON
        s->set_awb_gain(s, 1);      // Auto white balance gain ON
        s->set_exposure_ctrl(s, 1); // Auto exposure ON
        s->set_aec2(s, 1);          // Auto exposure correction ON
        s->set_gain_ctrl(s, 1);     // Auto gain ON
        s->set_agc_gain(s, 0);      // AGC gain (0-30), 0 = auto
        s->set_gainceiling(s, (gainceiling_t)6); // Gain ceiling 64x
        
        // Image enhancements
        s->set_brightness(s, 0);    // Brightness (-2 to 2)
        s->set_contrast(s, 0);      // Contrast (-2 to 2)
        s->set_saturation(s, 0);    // Saturation (-2 to 2)
        s->set_sharpness(s, 0);     // Sharpness (-2 to 2)
        s->set_denoise(s, 1);       // Denoise ON
        
        // Quality settings
        s->set_quality(s, 10);      // JPEG quality (0-63, lower is better)
        s->set_colorbar(s, 0);      // Disable color bar test pattern
        
        // Special features
        s->set_hmirror(s, 0);       // Horizontal mirror OFF
        s->set_vflip(s, 0);         // Vertical flip OFF
        s->set_lenc(s, 1);          // Lens correction ON
        
        Serial.println("[OK] Camera configured for optimal quality");
    }

    Serial.println("[OK] Camera initialized");
    return true;
}

// ============================================================
// INIT WIFI
// ============================================================

bool initWiFi()
{
    // Note: WiFi mode already set to WIFI_STA in initESPNow()
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("[WIFI] Connecting");
    int timeout = 30; // 30 seconds timeout
    while (WiFi.status() != WL_CONNECTED && timeout > 0)
    {
        delay(1000);
        Serial.print(".");
        timeout--;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print("[WIFI] Connected! IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    else
    {
        Serial.println("[WIFI] Connection failed!");
        return false;
    }
}

// ============================================================
// ESP-NOW CALLBACKS (ESP-IDF 5.x API)
// ============================================================

void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status)
{
    Serial.print("[ESP-NOW] Send status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len)
{
    // Copy data vào local struct
    struct_message tempMsg;
    memcpy(&tempMsg, data, sizeof(tempMsg));

    Serial.print("[ESP-NOW] Received command: ");
    Serial.println(tempMsg.command);

    // Xử lý command
    if (strcmp(tempMsg.command, "CAPTURE") == 0)
    {
        Serial.println("[CMD] CAPTURE request received");
        // ← KHÔNG gọi HTTP ở đây! Chỉ set flag
        captureRequested = true;
    }
    else if (strcmp(tempMsg.command, "PING") == 0)
    {
        Serial.println("[CMD] PING received");
        sendResponse("PONG");
    }
    else if (strcmp(tempMsg.command, "STATUS") == 0)
    {
        char status[64];
        sprintf(status, "CAM:%s,WIFI:%s,ESPNOW:%s",
                cameraOK ? "OK" : "ERR",
                wifiOK ? "OK" : "ERR",
                espnowOK ? "OK" : "ERR");
        sendResponse(status);
    }
}

// ============================================================
// ESP-NOW INIT
// ============================================================

bool initESPNow()
{
    WiFi.mode(WIFI_STA);
    delay(100);

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    Serial.print("[ESP-NOW] My MAC Address: ");
    for (int i = 0; i < 6; i++)
    {
        Serial.printf("%02X", mac[i]);
        if (i < 5)
            Serial.print(":");
    }
    Serial.println();

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("[ERROR] ESP-NOW init failed!");
        return false;
    }

    esp_now_register_send_cb(onDataSent);
    esp_now_register_recv_cb(onDataRecv);

    memcpy(peerInfo.peer_addr, controllerMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("[ERROR] Failed to add peer!");
        return false;
    }

    return true;
}

// ============================================================
// SEND RESPONSE VIA ESP-NOW
// ============================================================

void sendResponse(const char *response)
{
    strcpy(outgoingMsg.response, response);
    outgoingMsg.command[0] = '\0';

    esp_err_t result = esp_now_send(controllerMAC, (uint8_t *)&outgoingMsg, sizeof(outgoingMsg));

    Serial.print("[ESP-NOW] Sending: ");
    Serial.print(response);
    Serial.print(" - ");
    Serial.println(result == ESP_OK ? "OK" : "FAIL");
}

// ============================================================
// CLASSIFY IMAGE (HTTPS VERSION)
// ============================================================

int classifyImage(camera_fb_t *fb)
{
    Serial.println("[HTTPS] Sending image to server...");

    // ============================================================
    // KẾT NỐI HTTPS - CHỈ KẾT NỐI KHI CẦN
    // ============================================================
    unsigned long t1 = millis();
    
    // Kiểm tra connection hiện tại
    if (!httpsClient.connected())
    {
        Serial.printf("[HTTPS] Connecting to: %s:%d\n", SERVER_HOST, SERVER_PORT);
        
        if (!httpsClient.connect(SERVER_HOST, SERVER_PORT))
        {
            Serial.println("[ERROR] HTTPS connection failed!");
            Serial.print("[DEBUG] WiFi status: ");
            Serial.println(WiFi.status());
            Serial.printf("[TIMING] Failed after %dms\n", millis() - t1);
            return 0;
        }
        
        Serial.printf("[TIMING] SSL handshake: %dms\n", millis() - t1);
    }
    else
    {
        Serial.println("[HTTPS] Reusing existing connection");
    }

    // ============================================================
    // GỬI HTTP REQUEST (qua kênh SSL)
    // ============================================================
    unsigned long t2 = millis();
    httpsClient.printf("POST %s HTTP/1.1\r\n", SERVER_PATH);
    httpsClient.printf("Host: %s\r\n", SERVER_HOST);
    httpsClient.println("Content-Type: image/jpeg");
    httpsClient.printf("Content-Length: %d\r\n", fb->len);
    httpsClient.println("Connection: keep-alive");  // ← KEEP ALIVE!
    httpsClient.println();

    // Send image data in chunks
    const int CHUNK_SIZE = 1024;
    size_t remaining = fb->len;
    size_t offset = 0;

    while (remaining > 0)
    {
        size_t chunkLen = min(remaining, (size_t)CHUNK_SIZE);
        size_t written = httpsClient.write(fb->buf + offset, chunkLen);

        if (written == 0)
        {
            Serial.println("[ERROR] Failed to write data!");
            httpsClient.stop();  // Đóng connection lỗi
            return 0;
        }

        offset += written;
        remaining -= written;
        yield(); // Let WiFi stack process
    }

    Serial.printf("[TIMING] Upload %d bytes: %dms\n", fb->len, millis() - t2);

    // ============================================================
    // ĐỌC RESPONSE
    // ============================================================
    unsigned long t3 = millis();
    unsigned long timeout = millis() + 15000; // 15s timeout cho HTTPS
    while (!httpsClient.available() && millis() < timeout)
    {
        delay(10);
    }

    if (!httpsClient.available())
    {
        Serial.println("[ERROR] Server response timeout!");
        Serial.printf("[TIMING] Waited: %dms\n", millis() - t3);
        httpsClient.stop();  // Đóng connection timeout
        return 0;
    }

    Serial.printf("[TIMING] Wait for response: %dms\n", millis() - t3);

    // ============================================================
    // ĐỌC HEADERS VÀ LẤY CONTENT-LENGTH
    // ============================================================
    unsigned long t4 = millis();
    int contentLength = -1;
    int statusCode = 0;
    
    while (httpsClient.available())
    {
        String line = httpsClient.readStringUntil('\n');
        line.trim();
        
        // Parse status code
        if (line.startsWith("HTTP/1.1"))
        {
            statusCode = line.substring(9, 12).toInt();
            Serial.printf("[HTTP] Status: %d\n", statusCode);
        }
        
        // Parse Content-Length
        if (line.startsWith("Content-Length:"))
        {
            contentLength = line.substring(15).toInt();
            Serial.printf("[HTTP] Content-Length: %d\n", contentLength);
        }
        
        // End of headers
        if (line.length() == 0)
        {
            break;
        }
    }

    // ============================================================
    // ĐỌC JSON BODY (chính xác Content-Length bytes)
    // ============================================================
    String jsonResponse = "";
    
    if (contentLength > 0)
    {
        // Đọc chính xác contentLength bytes
        char *buffer = (char *)malloc(contentLength + 1);
        if (buffer)
        {
            int bytesRead = 0;
            unsigned long readStart = millis();
            
            while (bytesRead < contentLength && (millis() - readStart) < 3000)
            {
                if (httpsClient.available())
                {
                    buffer[bytesRead++] = httpsClient.read();
                }
                else
                {
                    delay(1);
                }
            }
            
            buffer[bytesRead] = '\0';
            jsonResponse = String(buffer);
            free(buffer);
            
            Serial.printf("[HTTP] Read %d/%d bytes\n", bytesRead, contentLength);
        }
    }
    else
    {
        // Fallback: đọc cho đến khi không còn data (tối đa 3s)
        Serial.println("[HTTP] No Content-Length, reading until timeout...");
        unsigned long readStart = millis();
        
        while ((millis() - readStart) < 3000)
        {
            if (httpsClient.available())
            {
                jsonResponse += (char)httpsClient.read();
                readStart = millis(); // Reset timeout khi có data
            }
            else
            {
                delay(10);
                // Nếu không có data trong 100ms, coi như xong
                if ((millis() - readStart) > 100)
                {
                    break;
                }
            }
        }
    }
    
    // ← KHÔNG GỌI stop() - giữ connection!

    Serial.printf("[TIMING] Read response: %dms\n", millis() - t4);
    Serial.print("[HTTPS] Response: ");
    Serial.println(jsonResponse);

    // ============================================================
    // PARSE JSON
    // ============================================================
    unsigned long t5 = millis();
    // Buffer size analysis:
    // - Success response: ~200 bytes
    // - Validation errors: ~500 bytes  
    // - No detection: ~600 bytes
    // - Buffer: 1024 bytes (1KB) = 70% safety margin
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, jsonResponse);

    if (error)
    {
        Serial.print("[ERROR] JSON parse error: ");
        Serial.println(error.c_str());
        return 0;
    }

    // Check success
    bool success = doc["success"] | false;
    if (!success)
    {
        const char *errMsg = doc["error"] | "Unknown error";
        Serial.print("[ERROR] Server error: ");
        Serial.println(errMsg);
        return 0;
    }

    // Get bin number
    int binNumber = doc["data"]["bin"] | 0;
    const char *className = doc["data"]["class"] | "unknown";
    float confidence = doc["data"]["confidence"] | 0.0f;

    Serial.printf("[TIMING] JSON parse: %dms\n", millis() - t5);
    Serial.printf("[RESULT] Class: %s, Bin: %d, Confidence: %.2f%%\n",
                  className, binNumber, confidence * 100);

    return binNumber;
}
