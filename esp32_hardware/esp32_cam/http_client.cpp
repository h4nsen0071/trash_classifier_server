/**
 * @file http_client.cpp
 * @brief HTTP Client Implementation
 */

#include "http_client.h"
#include "config.h"
#include "wifi_manager.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ============================================================
// PRIVATE VARIABLES
// ============================================================

static unsigned long lastRequestTime = 0;

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
        Serial.println(responseBody.substring(0, 200)); // First 200 chars
    #endif
    
    // Parse JSON
    StaticJsonDocument<512> doc;
    DeserializationError jsonError = deserializeJson(doc, responseBody);
    
    if (jsonError) {
        result.error = "JSON parse error: " + String(jsonError.c_str());
        LOG_HTTP_VAL("JSON error: ", jsonError.c_str());
        return result;
    }
    
    // Check for error in response
    if (doc.containsKey("error")) {
        result.error = doc["error"].as<String>();
        LOG_HTTP_VAL("Server error: ", result.error);
        return result;
    }
    
    // Extract classification data
    if (doc.containsKey("class") || doc.containsKey("predicted_class")) {
        result.className = doc["class"] | doc["predicted_class"].as<String>();
    }
    
    if (doc.containsKey("confidence")) {
        result.confidence = doc["confidence"].as<float>();
    }
    
    if (doc.containsKey("bin") || doc.containsKey("bin_number")) {
        result.binNumber = doc["bin"] | doc["bin_number"].as<int>();
    } else {
        // Map class to bin if not provided
        result.binNumber = mapClassToBin(result.className);
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
    
    if (result.confidence < MIN_CONFIDENCE) {
        result.error = "Low confidence: " + String(result.confidence);
        LOG_HTTP_VAL("Low confidence: ", result.confidence);
        return result;
    }
    
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
    LOG_HTTP_VAL("Server URL: ", SERVER_URL);
    LOG_HTTP_VAL("Timeout: ", HTTP_TIMEOUT_MS);
}

ClassificationResult httpClient_classify(const String& imageBase64) {
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
    
    LOG_HTTP("Sending classification request...");
    LOG_HTTP_VAL("Image base64 size: ", imageBase64.length());
    
    unsigned long startTime = millis();
    
    HTTPClient http;
    
    // Begin connection
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(HTTP_TIMEOUT_MS);
    
    // Build JSON payload
    String payload = "{\"image\":\"" + imageBase64 + "\"}";
    
    LOG_HTTP_VAL("Payload size: ", payload.length());
    
    // Send POST request
    int httpCode = http.POST(payload);
    
    lastRequestTime = millis() - startTime;
    LOG_HTTP_VAL("Request time (ms): ", lastRequestTime);
    LOG_HTTP_VAL("HTTP code: ", httpCode);
    
    if (httpCode <= 0) {
        result.error = "Connection failed: " + http.errorToString(httpCode);
        LOG_HTTP_VAL("Connection error: ", http.errorToString(httpCode));
        http.end();
        return result;
    }
    
    if (httpCode != 200) {
        result.error = "HTTP error: " + String(httpCode);
        LOG_HTTP_VAL("HTTP error code: ", httpCode);
        http.end();
        return result;
    }
    
    // Get response
    String responseBody = http.getString();
    LOG_HTTP_VAL("Response length: ", responseBody.length());
    
    http.end();
    
    // Parse response
    result = parseResponse(responseBody);
    
    return result;
}

bool httpClient_ping() {
    if (!wifiManager_isConnected()) {
        return false;
    }
    
    // Try to reach health endpoint
    HTTPClient http;
    String healthUrl = String(SERVER_URL);
    healthUrl.replace("/classify", "/health");
    
    http.begin(healthUrl);
    http.setTimeout(5000);
    
    int httpCode = http.GET();
    http.end();
    
    return (httpCode == 200);
}

unsigned long httpClient_getLastRequestTime() {
    return lastRequestTime;
}
