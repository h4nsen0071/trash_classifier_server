/**
 * @file esp32_controller.ino
 * @brief Smart Bin Controller - Main Entry Point
 * 
 * ESP32 Controller quản lý:
 * - Cảm biến khoảng cách HC-SR04
 * - 3 Servo motors (Paper, Plastic, Glass bins)
 * - LED status indicator
 * - Serial communication với ESP32-CAM
 * 
 * @author SmartBin Team
 * @version 1.0.0
 */

// ============================================================
// INCLUDES
// ============================================================

#include "config.h"
#include "pins.h"
#include "state_machine.h"
#include "distance_sensor.h"
#include "servo_controller.h"
#include "led_status.h"
#include "serial_comm.h"
#include "lcd_display.h"

// ============================================================
// GLOBAL VARIABLES
// ============================================================

// Current system state
static SystemState currentState = STATE_IDLE;
static SystemState previousState = STATE_IDLE;

// Timing variables
static unsigned long stateStartTime = 0;
static unsigned long lastSensorRead = 0;

// Stability check
static int stabilityCount = 0;

// Current bin being processed
static int currentBin = 0;

// ============================================================
// STATE MANAGEMENT
// ============================================================

void changeState(SystemState newState) {
    if (currentState != newState) {
        previousState = currentState;
        currentState = newState;
        stateStartTime = millis();
        
        #if defined(DEBUG_ENABLED) && defined(DEBUG_STATE)
            Serial.print("[STATE] ");
            Serial.print(getStateName(previousState));
            Serial.print(" → ");
            Serial.println(getStateName(newState));
        #endif
    }
}

unsigned long getTimeInState() {
    return millis() - stateStartTime;
}

// Flag để track LCD đã cập nhật chưa (reset khi đổi state)
static bool lcdIdleUpdated = false;

// ============================================================
// STATE HANDLERS
// ============================================================

void handleStateIdle() {
    ledStatus_setPattern(LED_OFF);
    
    // Hiển thị trạng thái chờ (chỉ cập nhật 1 lần khi vào IDLE)
    if (!lcdIdleUpdated) {
        lcdDisplay_showIdle();
        lcdIdleUpdated = true;
    }
    
    // Đọc sensor định kỳ
    if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
        lastSensorRead = millis();
        
        if (distanceSensor_objectDetected()) {
            stabilityCount = 1;
            lcdIdleUpdated = false;  // Reset flag để lần sau vào IDLE sẽ cập nhật LCD
            changeState(STATE_OBJECT_DETECTED);
        }
    }
}

void handleStateObjectDetected() {
    ledStatus_setPattern(LED_ON);
    
    // Kiểm tra vật thể còn ở đó không
    if (distanceSensor_objectDetected()) {
        stabilityCount++;
        
        #ifdef DEBUG_STATE
            Serial.print("[STATE] Stability count: ");
            Serial.print(stabilityCount);
            Serial.print("/");
            Serial.println(STABILITY_CHECKS);
        #endif
        
        if (stabilityCount >= STABILITY_CHECKS) {
            changeState(STATE_WAITING_STABLE);
        }
    } else {
        // Vật thể biến mất, quay về IDLE
        stabilityCount = 0;
        changeState(STATE_IDLE);
    }
    
    delay(STABILITY_DELAY_MS);
}

void handleStateWaitingStable() {
    ledStatus_setPattern(LED_BLINK_FAST);
    
    // Xác nhận lần cuối trước khi capture
    if (distanceSensor_objectDetected()) {
        changeState(STATE_REQUESTING_CAPTURE);
    } else {
        changeState(STATE_IDLE);
    }
}

void handleStateRequestingCapture() {
    ledStatus_setPattern(LED_BLINK_FAST);
    
    // Hiển thị đang xử lý
    lcdDisplay_showProcessing();
    
    // Clear buffer trước khi gửi
    serialComm_clearBuffer();
    
    // Gửi lệnh CAPTURE
    serialComm_sendCapture();
    
    changeState(STATE_WAITING_RESULT);
}

void handleStateWaitingResult() {
    ledStatus_setPattern(LED_BLINK_FAST);
    
    // Chờ response từ CAM
    CamResponseData response = serialComm_waitResponse(CAM_RESPONSE_TIMEOUT_MS);
    
    switch (response.type) {
        case RESPONSE_BIN:
            if (servoController_isValidBin(response.binNumber)) {
                currentBin = response.binNumber;
                changeState(STATE_OPENING_BIN);
            } else {
                Serial.print("[ERROR] Invalid bin number: ");
                Serial.println(response.binNumber);
                changeState(STATE_ERROR);
            }
            break;
            
        case RESPONSE_ERROR:
            Serial.print("[ERROR] CAM error: ");
            Serial.println(response.errorMessage);
            changeState(STATE_ERROR);
            break;
            
        case RESPONSE_TIMEOUT:
            Serial.println("[ERROR] CAM response timeout");
            changeState(STATE_ERROR);
            break;
            
        default:
            Serial.println("[ERROR] Unexpected response");
            changeState(STATE_ERROR);
            break;
    }
}

void handleStateOpeningBin() {
    ledStatus_setPattern(LED_ON);
    
    Serial.print("[ACTION] Opening bin ");
    Serial.print(currentBin);
    Serial.print(" (");
    Serial.print(servoController_getBinName(currentBin));
    Serial.println(")");
    
    // Hiển thị kết quả trên LCD
    lcdDisplay_showResult(
        servoController_getBinName(currentBin),
        servoController_getWasteType(currentBin)
    );
    
    // Mở bin
    if (servoController_openBin(currentBin)) {
        changeState(STATE_HOLDING_OPEN);
    } else {
        changeState(STATE_ERROR);
    }
}

void handleStateHoldingOpen() {
    ledStatus_setPattern(LED_ON);
    
    // Giữ nắp mở trong khoảng thời gian cấu hình
    if (getTimeInState() >= LID_OPEN_DURATION_MS) {
        changeState(STATE_CLOSING_BIN);
    }
}

void handleStateClosingBin() {
    Serial.print("[ACTION] Closing bin ");
    Serial.println(currentBin);
    
    // Đóng bin
    servoController_closeBin(currentBin);
    
    // Hiển thị success
    ledStatus_showSuccess();
    
    currentBin = 0;
    changeState(STATE_COOLDOWN);
}

void handleStateCooldown() {
    ledStatus_setPattern(LED_OFF);
    
    // Chờ cooldown
    if (getTimeInState() >= COOLDOWN_DURATION_MS) {
        // Kiểm tra đã clear chưa
        if (distanceSensor_isCleared()) {
            changeState(STATE_IDLE);
        }
        // Nếu chưa clear, tiếp tục chờ
    }
}

void handleStateError() {
    // Hiển thị error pattern
    ledStatus_showError();
    
    // Hiển thị lỗi trên LCD
    lcdDisplay_showError("Khong phan loai");
    
    // Log error state
    Serial.println("[ERROR] Error state - returning to IDLE");
    
    // Đảm bảo tất cả bins đóng
    servoController_closeAll();
    
    currentBin = 0;
    stabilityCount = 0;
    
    // Cooldown ngắn trước khi quay về IDLE
    delay(2000);
    changeState(STATE_IDLE);
}

// ============================================================
// SETUP
// ============================================================

void setup() {
    // Initialize debug serial (USB)
    Serial.begin(115200);
    delay(1000);
    
    Serial.println();
    Serial.println("========================================");
    Serial.println("    Smart Bin Controller v" FIRMWARE_VERSION);
    Serial.println("========================================");
    Serial.println();
    
    // Initialize modules
    Serial.println("[INIT] Starting initialization...");
    
    Serial.print("[INIT] Distance sensor... ");
    distanceSensor_init();
    Serial.println("OK");
    
    Serial.print("[INIT] Servo controller... ");
    servoController_init();
    Serial.println("OK");
    
    Serial.print("[INIT] LED status... ");
    ledStatus_init();
    Serial.println("OK");
    
    Serial.print("[INIT] Serial communication... ");
    serialComm_init();
    Serial.println("OK");
    
    Serial.print("[INIT] LCD display... ");
    if (lcdDisplay_init()) {
        Serial.println("OK");
        lcdDisplay_showStartup();
    } else {
        Serial.println("FAILED (continuing without LCD)");
    }
    
    // Startup indication
    ledStatus_showSuccess();
    
    Serial.println();
    Serial.println("[INIT] All systems ready!");
    Serial.println("[INIT] Waiting for objects...");
    Serial.println();
    
    changeState(STATE_IDLE);
}

// ============================================================
// MAIN LOOP
// ============================================================

void loop() {
    // Update LED (for non-blocking patterns)
    ledStatus_update();
    
    // State machine
    switch (currentState) {
        case STATE_IDLE:
            handleStateIdle();
            break;
            
        case STATE_OBJECT_DETECTED:
            handleStateObjectDetected();
            break;
            
        case STATE_WAITING_STABLE:
            handleStateWaitingStable();
            break;
            
        case STATE_REQUESTING_CAPTURE:
            handleStateRequestingCapture();
            break;
            
        case STATE_WAITING_RESULT:
            handleStateWaitingResult();
            break;
            
        case STATE_OPENING_BIN:
            handleStateOpeningBin();
            break;
            
        case STATE_HOLDING_OPEN:
            handleStateHoldingOpen();
            break;
            
        case STATE_CLOSING_BIN:
            handleStateClosingBin();
            break;
            
        case STATE_COOLDOWN:
            handleStateCooldown();
            break;
            
        case STATE_ERROR:
            handleStateError();
            break;
    }
}
