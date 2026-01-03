/**
 * @file http_client.cpp
 * @brief HTTP Client Implementation - Gửi raw JPEG (không dùng base64)
 */

#include "http_client.h"
#include "config.h"
#include "wifi_manager.h"
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// ============================================================
// PRIVATE VARIABLES
// ============================================================

static unsigned long lastRequestTime = 0;
static WiFiClientSecure httpsClient;

// ============================================================
// DEBUG LOGGING
// ============================================================

#if defined(DEBUG_ENABLED) && defined(DEBUG_HTTP)
    #define LOG_HTTP(msg) Serial.print("[HTTP] "); Serial.println(msg)
    #define LOG_HTTP_VAL(msg, val) Serial.print("[HTTP] "); Serial.print(msg); Serial.println(val)
#else
    #define LOG_HTTP(msg)
    #define LOG_HTTP_VAL(msg, val)
#endif

// ============================================================
// PRIVATE FUNCTIONS
// ============================================================

static int mapClassToBin(const String& className) {
    if (className == "paper") return 1;
    if (className == "plastic") return 2;
    if (className == "glass") return 3;
    return 0; // Unknown
}

static ClassificationResult parseResponse(const String& responseBody) {
    ClassificationResult result;
    result.success = false;
    result.className = "";
    result.binNumber = 0;
    result.confidence = 0.0;
    result.error = "";
    
    #if defined(DEBUG_ENABLED) && defined(DEBUG_JSON)
        Serial.print("[JSON] Parsing: ");
        Serial.println(responseBody.substring(0, 200));
    #endif
    
    // Parse JSON
    StaticJsonDocument<512> doc;
    DeserializationError jsonError = deserializeJson(doc, responseBody);
    
    if (jsonError) {
        result.error = "JSON parse error: " + String(jsonError.c_str());
        LOG_HTTP_VAL("JSON error: ", jsonError.c_str());
        return result;
    }
    
    // Check success field
    bool success = doc["success"] | false;
    if (!success) {
        // Lấy error message
        if (doc.containsKey("error")) {
            JsonObject error = doc["error"];
            result.error = error["message"] | "Unknown error";
        } else {
            result.error = "Request failed";
        }
        LOG_HTTP_VAL("Server error: ", result.error);
        return result;
    }
    
    // Extract from data object
    if (doc.containsKey("data")) {
        JsonObject data = doc["data"];
        result.className = data["class"] | "";
        result.confidence = data["confidence"] | 0.0;
        result.binNumber = data["bin"] | 0;
    }
    
    // Validate
    if (result.className.length() == 0) {
        result.error = "No class in response";
        return result;
    }
    
    if (result.binNumber < 1 || result.binNumber > 3) {
        result.error = "Invalid bin number";
        return result;
    }
    
    // Không check confidence ở đây - để server quyết định
    result.success = true;
    
    LOG_HTTP_VAL("Class: ", result.className);
    LOG_HTTP_VAL("Bin: ", result.binNumber);
    LOG_HTTP_VAL("Confidence: ", result.confidence);
    
    return result;
}

// ============================================================
// PUBLIC FUNCTIONS
// ============================================================

void httpClient_init() {
    LOG_HTTP("Initialized");
    LOG_HTTP_VAL("Server: ", SERVER_HOST);
    LOG_HTTP_VAL("Port: ", SERVER_PORT);
    LOG_HTTP_VAL("Path: ", SERVER_PATH);
    
    // Setup HTTPS client
    httpsClient.setInsecure();  // Skip certificate verification
}

ClassificationResult httpClient_classifyRaw(uint8_t* imageData, size_t imageLen) {
    ClassificationResult result;
    result.success = false;
    result.binNumber = 0;
    result.confidence = 0.0;
    
    // Check WiFi
    if (!wifiManager_isConnected()) {
        result.error = "WiFi disconnected";
        LOG_HTTP("ERROR: WiFi not connected");
        return result;
    }
    
    LOG_HTTP("Sending raw JPEG...");
    LOG_HTTP_VAL("Image size: ", imageLen);
    
    unsigned long startTime = millis();
    
    // Connect to server
    if (!httpsClient.connected()) {
        LOG_HTTP_VAL("Connecting to ", SERVER_HOST);
        
        if (!httpsClient.connect(SERVER_HOST, SERVER_PORT, HTTP_TIMEOUT_MS)) {
            result.error = "Connection failed";
            LOG_HTTP("ERROR: Connection failed");
            return result;
        }
    }
    
    // Send HTTP request
    httpsClient.printf("POST %s HTTP/1.1\r\n", SERVER_PATH);
    httpsClient.printf("Host: %s\r\n", SERVER_HOST);
    httpsClient.println("Content-Type: image/jpeg");
    httpsClient.printf("Content-Length: %d\r\n", imageLen);
    httpsClient.println("Connection: keep-alive");
    httpsClient.println();
    
    // Send image data in chunks
    size_t sent = 0;
    size_t chunkSize = 1024;
    while (sent < imageLen) {
        size_t toSend = min(chunkSize, imageLen - sent);
        httpsClient.write(imageData + sent, toSend);
        sent += toSend;
    }
    
    LOG_HTTP_VAL("Sent bytes: ", sent);
    
    // Read response
    String responseBody = "";
    bool headersDone = false;
    int httpCode = 0;
    
    unsigned long timeout = millis();
    while (httpsClient.connected() && millis() - timeout < HTTP_TIMEOUT_MS) {
        if (httpsClient.available()) {
            String line = httpsClient.readStringUntil('\n');
            
            // Parse HTTP status
            if (line.startsWith("HTTP/")) {
                int codeStart = line.indexOf(' ') + 1;
                httpCode = line.substring(codeStart, codeStart + 3).toInt();
                LOG_HTTP_VAL("HTTP code: ", httpCode);
            }
            
            // Empty line = headers done
            if (line == "\r" || line.length() == 0) {
                headersDone = true;
            }
            
            // Capture JSON body
            if (headersDone && line.startsWith("{")) {
                responseBody = line;
                break;
            }
        }
    }
    
    lastRequestTime = millis() - startTime;
    LOG_HTTP_VAL("Request time (ms): ", lastRequestTime);
    
    if (httpCode != 200) {
        result.error = "HTTP error: " + String(httpCode);
        return result;
    }
    
    if (responseBody.length() == 0) {
        result.error = "Empty response";
        return result;
    }
    
    // Parse response
    result = parseResponse(responseBody);
    
    return result;
}

// Legacy function - redirect to new raw function
ClassificationResult httpClient_classify(const String& imageBase64) {
    // Không dùng base64 nữa - chỉ để tương thích
    ClassificationResult result;
    result.success = false;
    result.error = "Use httpClient_classifyRaw() instead";
    return result;
}

bool httpClient_ping() {
    if (!wifiManager_isConnected()) {
        return false;
    }
    
    // Simple connection test
    WiFiClientSecure testClient;
    testClient.setInsecure();
    
    if (testClient.connect(SERVER_HOST, SERVER_PORT, 5000)) {
        testClient.stop();
        return true;
    }
    return false;
}

unsigned long httpClient_getLastRequestTime() {
    return lastRequestTime;
}
