/**
 * @file servo_controller.cpp
 * @brief Servo Motor Control Implementation
 */

#include "servo_controller.h"
#include "config.h"
#include "pins.h"
#include <ESP32Servo.h>

// ============================================================
// PRIVATE VARIABLES
// ============================================================

static Servo servoRed;      // Thùng đỏ (Bin 1)
static Servo servoGreen;    // Thùng xanh (Bin 2)
static Servo servoGray;     // Thùng xám (Bin 3)

static bool initialized = false;

// ============================================================
// DEBUG LOGGING
// ============================================================

#if defined(DEBUG_ENABLED) && defined(DEBUG_SERVO)
    #define LOG_SERVO(msg) Serial.print("[SERVO] "); Serial.println(msg)
    #define LOG_SERVO_VAL(msg, val) Serial.print("[SERVO] "); Serial.print(msg); Serial.println(val)
#else
    #define LOG_SERVO(msg)
    #define LOG_SERVO_VAL(msg, val)
#endif

// ============================================================
// PRIVATE FUNCTIONS
// ============================================================

static Servo* getServo(int binNumber) {
    switch (binNumber) {
        case BIN_RED:       return &servoRed;
        case BIN_GREEN:     return &servoGreen;
        case BIN_GRAY:      return &servoGray;
        default:            return nullptr;
    }
}

// ============================================================
// PUBLIC FUNCTIONS
// ============================================================

void servoController_init() {
    // Cho phép ESP32 servo sử dụng tất cả timers
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    
    // Attach servos với min/max pulse width
    servoRed.setPeriodHertz(50);
    servoRed.attach(PIN_SERVO_RED, 500, 2400);
    
    servoGreen.setPeriodHertz(50);
    servoGreen.attach(PIN_SERVO_GREEN, 500, 2400);
    
    servoGray.setPeriodHertz(50);
    servoGray.attach(PIN_SERVO_GRAY, 500, 2400);
    
    // Đóng tất cả
    servoController_closeAll();
    
    initialized = true;
    LOG_SERVO("All servos initialized");
    LOG_SERVO_VAL("Red (Bin1) on GPIO: ", PIN_SERVO_RED);
    LOG_SERVO_VAL("Green (Bin2) on GPIO: ", PIN_SERVO_GREEN);
    LOG_SERVO_VAL("Gray (Bin3) on GPIO: ", PIN_SERVO_GRAY);
}

bool servoController_openBin(int binNumber) {
    if (!initialized) {
        LOG_SERVO("ERROR: Not initialized");
        return false;
    }
    
    if (!servoController_isValidBin(binNumber)) {
        LOG_SERVO_VAL("ERROR: Invalid bin number: ", binNumber);
        return false;
    }
    
    Servo* servo = getServo(binNumber);
    if (servo == nullptr) return false;
    
    LOG_SERVO_VAL("Opening: ", servoController_getBinName(binNumber));
    LOG_SERVO_VAL("Waste type: ", servoController_getWasteType(binNumber));
    
    servo->write(SERVO_OPEN_ANGLE);
    delay(SERVO_MOVE_DELAY_MS);
    
    LOG_SERVO("Bin opened");
    return true;
}

bool servoController_closeBin(int binNumber) {
    if (!initialized) {
        LOG_SERVO("ERROR: Not initialized");
        return false;
    }
    
    if (!servoController_isValidBin(binNumber)) {
        LOG_SERVO_VAL("ERROR: Invalid bin number: ", binNumber);
        return false;
    }
    
    Servo* servo = getServo(binNumber);
    if (servo == nullptr) return false;
    
    LOG_SERVO_VAL("Closing: ", servoController_getBinName(binNumber));
    
    servo->write(SERVO_CLOSE_ANGLE);
    delay(SERVO_MOVE_DELAY_MS);
    
    LOG_SERVO("Bin closed");
    return true;
}

void servoController_closeAll() {
    LOG_SERVO("Closing all bins");
    
    servoRed.write(SERVO_CLOSE_ANGLE);
    servoGreen.write(SERVO_CLOSE_ANGLE);
    servoGray.write(SERVO_CLOSE_ANGLE);
    
    delay(SERVO_MOVE_DELAY_MS);
    
    LOG_SERVO("All bins closed");
}

bool servoController_isValidBin(int binNumber) {
    return (binNumber >= 1 && binNumber <= 3);
}

const char* servoController_getBinName(int binNumber) {
    switch (binNumber) {
        case BIN_RED:       return "Thung Do";      // Thùng Đỏ (ASCII)
        case BIN_GREEN:     return "Thung Xanh";    // Thùng Xanh
        case BIN_GRAY:      return "Thung Xam";     // Thùng Xám (ASCII)
        default:            return "Unknown";
    }
}

const char* servoController_getWasteType(int binNumber) {
    switch (binNumber) {
        case BIN_RED:       return "Giay/Paper";    // Giấy
        case BIN_GREEN:     return "Nhua/Plastic";  // Nhựa
        case BIN_GRAY:      return "Thuy tinh";     // Thủy tinh
        default:            return "Unknown";
    }
}
