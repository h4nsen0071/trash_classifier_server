/**
 * @file wifi_manager.cpp
 * @brief WiFi Connection Manager Implementation
 */

#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>

// ============================================================
// PRIVATE VARIABLES
// ============================================================

static WifiStatus currentStatus = WIFI_STATUS_DISCONNECTED;
static unsigned long lastReconnectAttempt = 0;
static int reconnectCount = 0;

// ============================================================
// DEBUG LOGGING
// ============================================================

#if defined(DEBUG_ENABLED) && defined(DEBUG_WIFI)
    #define LOG_WIFI(msg) Serial.print("[WIFI] "); Serial.println(msg)
    #define LOG_WIFI_VAL(msg, val) Serial.print("[WIFI] "); Serial.print(msg); Serial.println(val)
#else
    #define LOG_WIFI(msg)
    #define LOG_WIFI_VAL(msg, val)
#endif

// ============================================================
// PUBLIC FUNCTIONS
// ============================================================

bool wifiManager_init() {
    LOG_WIFI("Initializing...");
    
    // Set WiFi mode
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    
    // Connect
    return wifiManager_connect();
}

bool wifiManager_connect() {
    currentStatus = WIFI_STATUS_CONNECTING;
    
    LOG_WIFI_VAL("Connecting to: ", WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    unsigned long startTime = millis();
    int dotCount = 0;
    
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > WIFI_CONNECT_TIMEOUT_MS) {
            LOG_WIFI("Connection timeout!");
            currentStatus = WIFI_STATUS_FAILED;
            return false;
        }
        
        delay(500);
        
        #if defined(DEBUG_ENABLED) && defined(DEBUG_WIFI)
            Serial.print(".");
            dotCount++;
            if (dotCount % 20 == 0) Serial.println();
        #endif
    }
    
    #if defined(DEBUG_ENABLED) && defined(DEBUG_WIFI)
        Serial.println();
    #endif
    
    currentStatus = WIFI_STATUS_CONNECTED;
    reconnectCount = 0;
    
    LOG_WIFI("Connected!");
    LOG_WIFI_VAL("IP: ", WiFi.localIP().toString());
    LOG_WIFI_VAL("RSSI: ", WiFi.RSSI());
    LOG_WIFI_VAL("MAC: ", WiFi.macAddress());
    
    return true;
}

void wifiManager_disconnect() {
    LOG_WIFI("Disconnecting...");
    WiFi.disconnect(true);
    currentStatus = WIFI_STATUS_DISCONNECTED;
}

bool wifiManager_isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String wifiManager_getIP() {
    return WiFi.localIP().toString();
}

int wifiManager_getRSSI() {
    return WiFi.RSSI();
}

String wifiManager_getMAC() {
    return WiFi.macAddress();
}

bool wifiManager_reconnect() {
    if (reconnectCount >= WIFI_RECONNECT_ATTEMPTS) {
        LOG_WIFI("Max reconnect attempts reached");
        return false;
    }
    
    LOG_WIFI_VAL("Reconnecting... Attempt: ", reconnectCount + 1);
    
    WiFi.disconnect();
    delay(1000);
    
    bool success = wifiManager_connect();
    
    if (!success) {
        reconnectCount++;
    }
    
    return success;
}

void wifiManager_maintain() {
    if (!wifiManager_isConnected()) {
        if (millis() - lastReconnectAttempt > WIFI_RECONNECT_DELAY_MS) {
            lastReconnectAttempt = millis();
            wifiManager_reconnect();
        }
    }
}
