/**
 * @file serial_comm.cpp
 * @brief Serial Communication Implementation (CAM side)
 */

#include "serial_comm.h"
#include "config.h"
#include "protocol.h"

// ============================================================
// DEBUG LOGGING
// ============================================================

// Khi chạy thực tế, Serial dùng để giao tiếp với Controller
// nên không thể dùng để debug. Chỉ dùng Serial2 nếu có.
// Ở đây ta dùng Serial để giao tiếp VÀ debug (thay phiên)

#if defined(DEBUG_ENABLED) && defined(DEBUG_SERIAL_COMM)
    // Dùng Serial2 cho debug nếu có, hoặc log vào buffer
    #define LOG_SERIAL_COMM(msg) // Disabled in production
    #define LOG_SERIAL_COMM_VAL(msg, val)
#else
    #define LOG_SERIAL_COMM(msg)
    #define LOG_SERIAL_COMM_VAL(msg, val)
#endif

// ============================================================
// PUBLIC FUNCTIONS
// ============================================================

void serialComm_init() {
    // ESP32-CAM dùng UART0 (pins GPIO1/GPIO3)
    // Đây là Serial mặc định
    Serial.begin(SERIAL_BAUD);
    Serial.setTimeout(SERIAL_READ_TIMEOUT_MS);
    
    LOG_SERIAL_COMM("Serial communication initialized");
    LOG_SERIAL_COMM_VAL("Baud rate: ", SERIAL_BAUD);
}

ControllerCommand serialComm_checkCommand() {
    if (!Serial.available()) {
        return CMD_NONE;
    }
    
    String data = Serial.readStringUntil(MSG_TERMINATOR);
    data.trim();
    
    if (data.length() == 0) {
        return CMD_NONE;
    }
    
    LOG_SERIAL_COMM_VAL("RX: ", data);
    
    if (data == CMD_CAPTURE) {
        return CMD_CAPTURE_REQ;
    }
    
    if (data == CMD_PING) {
        return CMD_PING_REQ;
    }
    
    if (data == CMD_STATUS) {
        return CMD_STATUS_REQ;
    }
    
    // Unknown command
    LOG_SERIAL_COMM_VAL("Unknown command: ", data);
    return CMD_NONE;
}

void serialComm_sendBin(int binNumber) {
    String response = String(RESP_BIN_PREFIX) + String(binNumber);
    Serial.println(response);
    
    LOG_SERIAL_COMM_VAL("TX: ", response);
}

void serialComm_sendError(const char* errorCode) {
    String response = String(RESP_ERROR_PREFIX) + String(errorCode);
    Serial.println(response);
    
    LOG_SERIAL_COMM_VAL("TX: ", response);
}

void serialComm_sendPong() {
    Serial.println(RESP_PONG);
    LOG_SERIAL_COMM("TX: PONG");
}

void serialComm_sendReady() {
    Serial.println(RESP_READY);
    LOG_SERIAL_COMM("TX: READY");
}
