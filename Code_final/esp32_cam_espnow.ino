/**
 * @file esp32_cam_espnow.ino
 * @brief ESP32-CAM với ESP-NOW - Giao tiếp trực tiếp P2P với Controller
 *
 * ⚠️ QUAN TRỌNG - TRONG ARDUINO IDE:
 * 1. Tools > Board > AI Thinker ESP32-CAM
 * 2. Tools > PSRAM > Enabled  ← BẮT BUỘC!
 * 3. Tools > Partition Scheme > Huge APP (3MB No OTA)
 *
 * ESP-NOW: Giao tiếp WiFi trực tiếp giữa 2 ESP32
 * - KHÔNG CẦN router cho giao tiếp ESP-NOW
 * - KHÔNG CẦN MQTT broker
 * - KHÔNG CẦN PC
 * - Chỉ cần biết MAC address của nhau
 * - VẪN CẦN WiFi để gửi HTTP request đến Flask server
 */

#include "esp_camera.h"
#include <WiFi.h>
#include <esp_now.h>
#include <esp_mac.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ============================================================
// CẤU HÌNH - THAY ĐỔI Ở ĐÂY
// ============================================================

// WiFi (vẫn cần WiFi để gửi HTTP request đến Flask server)
const char *WIFI_SSID = "ZTE 2.4G";
const char *WIFI_PASSWORD = "";

// Server - Local testing
const char *SERVER_HOST = "192.168.2.5";
const int SERVER_PORT = 5000;
const char *SERVER_PATH = "/classify";

// ESP-NOW: MAC Address của ESP32 Controller
// ⚠️ QUAN TRỌNG: Phải lấy MAC của Controller và điền vào đây!
// Cách lấy: Upload code lên Controller, xem Serial Monitor khi boot
// Ví dụ: {0x24, 0x6F, 0x28, 0x12, 0x34, 0x56}
uint8_t controllerMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // ← THAY ĐỔI!

// ESP-NOW Message structure (phải giống với Controller)
typedef struct struct_message
{
    char command[32];  // "CAPTURE", "PING", "STATUS"
    char response[64]; // "BIN:1", "ERROR:xxx", "PONG", "READY"
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
bool captureRequested = false; // Flag để xử lý trong loop()

WiFiClient httpClient;
struct_message incomingMsg;
struct_message outgoingMsg;
esp_now_peer_info_t peerInfo;

// ============================================================
// FUNCTION DECLARATIONS
// ============================================================

bool initCamera();
bool initWiFi();
bool initESPNow();
void sendResponse(const char *response);
int classifyImage(camera_fb_t *fb);
void handleCapture();

// ============================================================
// CAMERA INIT
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

    // Set QVGA
    sensor_t *s = esp_camera_sensor_get();
    if (s)
    {
        s->set_framesize(s, FRAMESIZE_QVGA);
    }

    Serial.println("[OK] Camera initialized");
    return true;
}

// ============================================================
// WIFI
// ============================================================

bool initWiFi()
{
    Serial.print("[WiFi] Connecting to ");
    Serial.print(WIFI_SSID);

    // Không gọi WiFi.mode() ở đây vì ESP-NOW đã set
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int count = 0;
    while (WiFi.status() != WL_CONNECTED && count < 40)
    {
        delay(500);
        Serial.print(".");
        count++;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print("[OK] WiFi connected! IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    else
    {
        Serial.println("[ERROR] WiFi connection failed!");
        return false;
    }
}

// ============================================================
// ESP-NOW CALLBACKS
// ============================================================

// Callback khi gửi data (ESP-IDF 5.5 API - wifi_tx_info_t)
void onDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status)
{
    Serial.print("[ESP-NOW] Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

// Callback khi nhận data từ Controller (ESP-IDF 5.x API)
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len)
{
    memcpy(&incomingMsg, incomingData, sizeof(incomingMsg));

    Serial.print("[ESP-NOW] Received from ");
    for (int i = 0; i < 6; i++)
    {
        Serial.printf("%02X", info->src_addr[i]);
        if (i < 5)
            Serial.print(":");
    }
    Serial.print(" - Command: ");
    Serial.println(incomingMsg.command);

    // Xử lý command
    String cmd = String(incomingMsg.command);
    cmd.trim();

    if (cmd == "CAPTURE")
    {
        // KHÔNG gọi HTTP trong callback! Chỉ set flag
        captureRequested = true;
        Serial.println("[ESP-NOW] Capture requested, will process in loop()");
    }
    else if (cmd == "PING")
    {
        sendResponse("PONG");
    }
    else if (cmd == "STATUS")
    {
        if (cameraOK && wifiOK && espnowOK)
        {
            sendResponse("READY");
        }
        else
        {
            sendResponse("ERROR:NOT_READY");
        }
    }
}

// ============================================================
// ESP-NOW INIT
// ============================================================

bool initESPNow()
{
    // Set device as WiFi Station
    WiFi.mode(WIFI_STA);

    // Cần delay để WiFi driver khởi tạo xong
    delay(100);

    // Print MAC address (dùng esp_read_mac thay vì WiFi.macAddress)
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
    Serial.println(">>> COPY MAC nay vao Controller code! <<<");

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("[ERROR] ESP-NOW init failed!");
        return false;
    }
    Serial.println("[OK] ESP-NOW initialized");

    // Register callbacks (ESP-IDF 5.x API)
    esp_now_register_send_cb(onDataSent);
    esp_now_register_recv_cb(onDataRecv);

    // Add Controller as peer
    memcpy(peerInfo.peer_addr, controllerMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("[ERROR] Failed to add peer!");
        return false;
    }
    Serial.println("[OK] Controller peer added");

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
// CLASSIFY IMAGE
// ============================================================

int classifyImage(camera_fb_t *fb)
{
    Serial.println("[HTTP] Sending image to server...");
    Serial.print("[HTTP] Connecting to: ");
    Serial.print(SERVER_HOST);
    Serial.print(":");
    Serial.println(SERVER_PORT);

    if (httpClient.connected())
    {
        httpClient.stop();
    }

    if (!httpClient.connect(SERVER_HOST, SERVER_PORT))
    {
        Serial.println("[ERROR] HTTP connection failed!");
        Serial.print("[DEBUG] WiFi status: ");
        Serial.println(WiFi.status());
        Serial.print("[DEBUG] Local IP: ");
        Serial.println(WiFi.localIP());
        return 0;
    }

    // Send HTTP POST request
    httpClient.printf("POST %s HTTP/1.1\r\n", SERVER_PATH);
    httpClient.printf("Host: %s\r\n", SERVER_HOST);
    httpClient.println("Content-Type: image/jpeg");
    httpClient.printf("Content-Length: %d\r\n", fb->len);
    httpClient.println("Connection: close");
    httpClient.println();

    // Send image data
    size_t sent = 0;
    while (sent < fb->len)
    {
        size_t toSend = min((size_t)1024, fb->len - sent);
        size_t written = httpClient.write(fb->buf + sent, toSend);
        if (written == 0)
        {
            Serial.println("[ERROR] HTTP write failed!");
            httpClient.stop();
            return 0;
        }
        sent += written;
    }
    Serial.print("[HTTP] Sent ");
    Serial.print(sent);
    Serial.println(" bytes");

    // Read response
    String jsonBody = "";
    bool headersEnded = false;
    unsigned long timeout = millis();

    while (millis() - timeout < 30000)
    {
        if (!httpClient.connected() && !httpClient.available())
        {
            break;
        }

        if (httpClient.available())
        {
            String line = httpClient.readStringUntil('\n');

            if (line == "\r" || line.length() == 0)
            {
                headersEnded = true;
            }

            if (headersEnded && line.startsWith("{"))
            {
                jsonBody = line;
                break;
            }
        }
        delay(1);
    }

    httpClient.stop();

    if (jsonBody.length() == 0)
    {
        Serial.println("[ERROR] No JSON response from server!");
        return 0;
    }

    Serial.print("[HTTP] Response: ");
    Serial.println(jsonBody);

    // Parse JSON
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, jsonBody);

    if (error)
    {
        Serial.print("[ERROR] JSON parse failed: ");
        Serial.println(error.c_str());
        return 0;
    }

    if (!doc.containsKey("success") || !doc["success"].as<bool>())
    {
        Serial.println("[ERROR] Server returned success=false");
        return 0;
    }

    if (!doc.containsKey("data") || !doc["data"].containsKey("bin"))
    {
        Serial.println("[ERROR] Missing bin number in response");
        return 0;
    }

    int binNumber = doc["data"]["bin"].as<int>();
    Serial.print("[OK] Classified as bin ");
    Serial.println(binNumber);

    return binNumber;
}

// ============================================================
// HANDLE CAPTURE
// ============================================================

void handleCapture()
{
    Serial.println("[CAPTURE] Starting...");

    if (!cameraOK)
    {
        Serial.println("[ERROR] Camera not ready!");
        sendResponse("ERROR:CAMERA_INIT_FAILED");
        return;
    }

    if (!wifiOK)
    {
        Serial.println("[ERROR] WiFi not connected!");
        sendResponse("ERROR:WIFI_DISCONNECTED");
        return;
    }

    // Capture image
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial.println("[ERROR] Camera capture failed!");
        sendResponse("ERROR:CAPTURE_FAILED");
        return;
    }
    Serial.print("[CAPTURE] Image size: ");
    Serial.print(fb->len);
    Serial.println(" bytes");

    // Classify
    int bin = classifyImage(fb);

    // Release frame buffer
    esp_camera_fb_return(fb);

    // Send result via ESP-NOW
    if (bin >= 1 && bin <= 3)
    {
        char response[20];
        sprintf(response, "BIN:%d", bin);
        sendResponse(response);
    }
    else
    {
        sendResponse("ERROR:SERVER_ERROR");
    }
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    // Serial cho debug (qua USB)
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("========================================");
    Serial.println("ESP32-CAM with ESP-NOW - Smart Trash Bin");
    Serial.println("========================================");

    // Init camera
    Serial.println("[INIT] Initializing camera...");
    cameraOK = initCamera();
    if (!cameraOK)
    {
        Serial.println("[FATAL] Camera init failed! Halting.");
        while (1)
        {
            delay(1000);
        }
    }

    // Init ESP-NOW (TRƯỚC WiFi.begin để lấy MAC)
    Serial.println("[INIT] Initializing ESP-NOW...");
    espnowOK = initESPNow();
    if (!espnowOK)
    {
        Serial.println("[FATAL] ESP-NOW init failed! Halting.");
        while (1)
        {
            delay(1000);
        }
    }

    // Init WiFi (để kết nối server)
    Serial.println("[INIT] Connecting to WiFi...");
    wifiOK = initWiFi();
    if (!wifiOK)
    {
        Serial.println("[WARNING] WiFi connection failed! Will retry in loop.");
    }

    Serial.println("========================================");
    Serial.println("[READY] ESP32-CAM is ready!");
    Serial.println("========================================");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // Maintain WiFi
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[WiFi] Disconnected! Reconnecting...");
        wifiOK = false;
        WiFi.reconnect();
        delay(5000);
        return;
    }
    wifiOK = true;

    // Xử lý capture request (NGOÀI callback)
    if (captureRequested)
    {
        captureRequested = false;
        Serial.println("[LOOP] Processing capture request...");
        handleCapture();
    }

    delay(10);
}
