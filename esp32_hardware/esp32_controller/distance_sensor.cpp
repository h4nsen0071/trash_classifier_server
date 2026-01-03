/**
 * @file distance_sensor.cpp
 * @brief HC-SR04 Distance Sensor Implementation
 */

#include "distance_sensor.h"
#include "config.h"
#include "pins.h"

// ============================================================
// PRIVATE VARIABLES
// ============================================================

static int lastDistance = -1;

// ============================================================
// DEBUG LOGGING
// ============================================================

#if defined(DEBUG_ENABLED) && defined(DEBUG_DISTANCE)
    #define LOG_DISTANCE(msg) Serial.print("[DISTANCE] "); Serial.println(msg)
    #define LOG_DISTANCE_VAL(msg, val) Serial.print("[DISTANCE] "); Serial.print(msg); Serial.println(val)
#else
    #define LOG_DISTANCE(msg)
    #define LOG_DISTANCE_VAL(msg, val)
#endif

// ============================================================
// PUBLIC FUNCTIONS
// ============================================================

void distanceSensor_init() {
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    digitalWrite(PIN_TRIG, LOW);
    
    LOG_DISTANCE("Initialized");
}

int distanceSensor_read() {
    // Gửi trigger pulse
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);
    
    // Đọc echo pulse với timeout
    unsigned long duration = pulseIn(PIN_ECHO, HIGH, 30000); // 30ms timeout
    
    if (duration == 0) {
        LOG_DISTANCE("Timeout - no echo received");
        lastDistance = -1;
        return -1;
    }
    
    // Tính khoảng cách: distance = (duration * 0.034) / 2
    int distance = duration * 0.034 / 2;
    
    // Validate range
    if (distance < 2 || distance > MAX_DISTANCE_CM) {
        LOG_DISTANCE_VAL("Out of range: ", distance);
        lastDistance = -1;
        return -1;
    }
    
    lastDistance = distance;
    
    #if defined(DEBUG_ENABLED) && defined(DEBUG_DISTANCE)
        static unsigned long lastLogTime = 0;
        if (millis() - lastLogTime > 500) { // Log mỗi 500ms để không spam
            LOG_DISTANCE_VAL("Distance: ", distance);
            lastLogTime = millis();
        }
    #endif
    
    return distance;
}

bool distanceSensor_objectDetected() {
    int distance = distanceSensor_read();
    if (distance == -1) return false;
    
    bool detected = (distance < OBJECT_DISTANCE_CM);
    
    if (detected) {
        LOG_DISTANCE_VAL("Object detected at ", distance);
    }
    
    return detected;
}

bool distanceSensor_isCleared() {
    int distance = distanceSensor_read();
    if (distance == -1) return true; // Assume cleared if can't read
    
    return (distance > OBJECT_CLEAR_CM);
}
