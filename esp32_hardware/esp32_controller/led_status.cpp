/**
 * @file led_status.cpp
 * @brief LED Status Indicator Implementation
 */

#include "led_status.h"
#include "config.h"
#include "pins.h"

// ============================================================
// PRIVATE VARIABLES
// ============================================================

static LedPattern currentPattern = LED_OFF;
static unsigned long lastToggleTime = 0;
static bool ledState = false;

// ============================================================
// DEBUG LOGGING
// ============================================================

#if defined(DEBUG_ENABLED) && defined(DEBUG_LED)
    #define LOG_LED(msg) Serial.print("[LED] "); Serial.println(msg)
#else
    #define LOG_LED(msg)
#endif

// ============================================================
// PRIVATE FUNCTIONS
// ============================================================

static void setLedState(bool on) {
    ledState = on;
    digitalWrite(PIN_LED_STATUS, on ? HIGH : LOW);
}

// ============================================================
// PUBLIC FUNCTIONS
// ============================================================

void ledStatus_init() {
    pinMode(PIN_LED_STATUS, OUTPUT);
    setLedState(false);
    currentPattern = LED_OFF;
    
    LOG_LED("Initialized");
}

void ledStatus_setPattern(LedPattern pattern) {
    if (currentPattern != pattern) {
        currentPattern = pattern;
        lastToggleTime = millis();
        
        #if defined(DEBUG_ENABLED) && defined(DEBUG_LED)
            const char* patternName;
            switch (pattern) {
                case LED_OFF:           patternName = "OFF"; break;
                case LED_ON:            patternName = "ON"; break;
                case LED_BLINK_FAST:    patternName = "BLINK_FAST"; break;
                case LED_BLINK_SLOW:    patternName = "BLINK_SLOW"; break;
                case LED_PULSE:         patternName = "PULSE"; break;
                default:                patternName = "UNKNOWN"; break;
            }
            Serial.print("[LED] Pattern: ");
            Serial.println(patternName);
        #endif
    }
}

void ledStatus_update() {
    unsigned long currentTime = millis();
    unsigned long interval;
    
    switch (currentPattern) {
        case LED_OFF:
            setLedState(false);
            break;
            
        case LED_ON:
            setLedState(true);
            break;
            
        case LED_BLINK_FAST:
            interval = LED_BLINK_FAST_MS;
            if (currentTime - lastToggleTime >= interval) {
                setLedState(!ledState);
                lastToggleTime = currentTime;
            }
            break;
            
        case LED_BLINK_SLOW:
            interval = LED_BLINK_SLOW_MS;
            if (currentTime - lastToggleTime >= interval) {
                setLedState(!ledState);
                lastToggleTime = currentTime;
            }
            break;
            
        case LED_PULSE:
            // Simple pulse effect using PWM simulation
            // Nếu board hỗ trợ analogWrite, có thể dùng PWM thực
            interval = 50;
            if (currentTime - lastToggleTime >= interval) {
                setLedState(!ledState);
                lastToggleTime = currentTime;
            }
            break;
    }
}

void ledStatus_blinkTimes(int count, int onTime, int offTime) {
    LOG_LED("Blinking...");
    
    for (int i = 0; i < count; i++) {
        setLedState(true);
        delay(onTime);
        setLedState(false);
        delay(offTime);
    }
}

void ledStatus_showSuccess() {
    LOG_LED("Success pattern");
    ledStatus_blinkTimes(3, 100, 100);
}

void ledStatus_showError() {
    LOG_LED("Error pattern");
    ledStatus_blinkTimes(LED_ERROR_BLINK_COUNT, 200, 200);
}
