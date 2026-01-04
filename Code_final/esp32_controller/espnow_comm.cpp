/**
 * @file espnow_comm.cpp
 * @brief ESP-NOW Communication Implementation (Controller side)
 *
 * ESP-NOW cho phép 2 ESP32 giao tiếp trực tiếp qua WiFi
 * mà không cần router hay broker
 */

#include "espnow_comm.h"
#include "config.h"
#include <WiFi.h>
#include <esp_now.h>
#include <esp_mac.h>

// ============================================================
// ESP-NOW MESSAGE STRUCTURE
// Phải giống với ESP32-CAM
// ============================================================

typedef struct struct_message
{
    char command[32];  // "CAPTURE", "PING", "STATUS"
    char response[64]; // "BIN:1", "ERROR:xxx", "PONG", "READY"
} struct_message;

// ============================================================
// CONFIGURATION
// ============================================================

// MAC Address của ESP32-CAM
// ⚠️ THAY ĐỔI SAU KHI LẤY MAC TỪ ESP32-CAM!
// Mở Serial Monitor của ESP32-CAM, copy MAC address hiển thị
uint8_t camMAC[] = {0xA0, 0xDD, 0x6C, 0xA3, 0x2A, 0xE4};

// ============================================================
// PRIVATE VARIABLES
// ============================================================

static struct_message outgoingMsg;
static struct_message incomingMsg;
static esp_now_peer_info_t peerInfo;

// Response buffer
static volatile bool responseReceived = false;
static CamResponseData lastResponse; // Không dùng volatile với struct phức tạp

// ============================================================
// DEBUG LOGGING
// ============================================================

#if defined(DEBUG_ENABLED) && defined(DEBUG_SERIAL_COMM)
#define LOG_ESPNOW(msg)         \
    Serial.print("[ESP-NOW] "); \
    Serial.println(msg)
#define LOG_ESPNOW_VAL(msg, val) \
    Serial.print("[ESP-NOW] ");  \
    Serial.print(msg);           \
    Serial.println(val)
#else
#define LOG_ESPNOW(msg)
#define LOG_ESPNOW_VAL(msg, val)
#endif

// ============================================================
// CALLBACK FUNCTIONS
// ============================================================

// Callback khi gửi data (ESP-IDF 5.5 API - wifi_tx_info_t)
void onDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status)
{
    Serial.print("[ESP-NOW] Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

// Callback khi nhận data từ CAM (ESP-IDF 5.x API)
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len)
{
    memcpy(&incomingMsg, incomingData, sizeof(incomingMsg));

    Serial.print("[ESP-NOW] RX: ");
    Serial.println(incomingMsg.response);

    // Parse response
    String data = String(incomingMsg.response);
    data.trim();

    CamResponseData response;
    response.type = RESPONSE_NONE;
    response.binNumber = 0;
    response.errorMessage = "";

    // Check for BIN response: "BIN:1", "BIN:2", "BIN:3"
    if (data.startsWith("BIN:"))
    {
        response.type = RESPONSE_BIN;
        response.binNumber = data.substring(4).toInt();
        Serial.print("[ESP-NOW] Parsed bin: ");
        Serial.println(response.binNumber);
    }
    // Check for ERROR response: "ERROR:message"
    else if (data.startsWith("ERROR:"))
    {
        response.type = RESPONSE_ERROR;
        response.errorMessage = data.substring(6);
        Serial.print("[ESP-NOW] Parsed error: ");
        Serial.println(response.errorMessage);
    }
    // Check for PONG response
    else if (data == "PONG")
    {
        response.type = RESPONSE_PONG;
        Serial.println("[ESP-NOW] Parsed PONG");
    }

    // Store response (copy từng field)
    lastResponse.type = response.type;
    lastResponse.binNumber = response.binNumber;
    lastResponse.errorMessage = response.errorMessage;
    responseReceived = true;
}

// ============================================================
// PUBLIC FUNCTIONS
// ============================================================

bool espnowComm_init()
{
    // Set WiFi mode to Station
    WiFi.mode(WIFI_STA);
    delay(100); // Wait for WiFi driver

    // Print MAC address (dùng esp_read_mac)
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    Serial.print("[ESP-NOW] Controller MAC: ");
    for (int i = 0; i < 6; i++)
    {
        Serial.printf("%02X", mac[i]);
        if (i < 5)
            Serial.print(":");
    }
    Serial.println();
    Serial.println(">>> COPY MAC nay vao ESP32-CAM code! <<<");

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("[ESP-NOW] Init failed!");
        return false;
    }
    Serial.println("[ESP-NOW] Initialized");

    // Register callbacks
    esp_now_register_send_cb(onDataSent);
    esp_now_register_recv_cb(onDataRecv);

    // Add ESP32-CAM as peer
    memcpy(peerInfo.peer_addr, camMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("[ESP-NOW] Failed to add CAM peer!");
        return false;
    }
    Serial.println("[ESP-NOW] CAM peer added");

    // Clear response buffer
    responseReceived = false;
    lastResponse.type = RESPONSE_NONE;

    return true;
}

void espnowComm_sendCapture()
{
    Serial.println("[ESP-NOW] TX: CAPTURE");

    // Clear response buffer trước khi gửi
    responseReceived = false;
    lastResponse.type = RESPONSE_NONE;

    // Prepare message
    strcpy(outgoingMsg.command, "CAPTURE");
    outgoingMsg.response[0] = '\0';

    // Send
    Serial.println("[ESP-NOW] >>> Sending CAPTURE command...");
    unsigned long sendTime = millis();
    esp_err_t result = esp_now_send(camMAC, (uint8_t *)&outgoingMsg, sizeof(outgoingMsg));

    if (result == ESP_OK)
    {
        Serial.println("[ESP-NOW] >>> Command sent OK");
    }
    else
    {
        Serial.printf("[ESP-NOW] >>> Send FAILED (error: 0x%x)\n", result);
    }
}

void espnowComm_sendPing()
{
    LOG_ESPNOW("TX: PING");

    strcpy(outgoingMsg.command, "PING");
    outgoingMsg.response[0] = '\0';

    esp_now_send(camMAC, (uint8_t *)&outgoingMsg, sizeof(outgoingMsg));
}

CamResponseData espnowComm_checkResponse()
{
    CamResponseData response;
    response.type = RESPONSE_NONE;
    response.binNumber = 0;
    response.errorMessage = "";

    if (responseReceived)
    {
        response = lastResponse;
        responseReceived = false;
    }

    return response;
}

CamResponseData espnowComm_waitResponse(unsigned long timeoutMs)
{
    CamResponseData response;
    response.type = RESPONSE_NONE;
    response.binNumber = 0;
    response.errorMessage = "";

    Serial.print("[ESP-NOW] === WAITING for response (timeout: ");
    Serial.print(timeoutMs);
    Serial.println("ms) ===");

    unsigned long startTime = millis();

    while (millis() - startTime < timeoutMs)
    {
        if (responseReceived)
        {
            unsigned long elapsed = millis() - startTime;
            response = lastResponse;
            responseReceived = false;
            
            Serial.print("[ESP-NOW] ✓ Response received after ");
            Serial.print(elapsed);
            Serial.println("ms");
            
            return response;
        }
        
        // Print progress mỗi 2 giây
        if ((millis() - startTime) % 2000 < 10) {
            Serial.print(".");
        }
        
        delay(10);
    }

    // Timeout
    unsigned long elapsed = millis() - startTime;
    Serial.print("[ESP-NOW] ✗ TIMEOUT after ");
    Serial.print(elapsed);
    Serial.println("ms");
    
    response.type = RESPONSE_TIMEOUT;
    return response;
}

void espnowComm_clearBuffer()
{
    responseReceived = false;
    lastResponse.type = RESPONSE_NONE;
    lastResponse.binNumber = 0;
    lastResponse.errorMessage = "";
    LOG_ESPNOW("Buffer cleared");
}

bool espnowComm_available()
{
    return responseReceived;
}
