/**
 * @file serial_comm.cpp
 * @brief Serial Communication Implementation (Controller side)
 */

#include "serial_comm.h"
#include "config.h"
#include "pins.h"
#include "protocol.h"

// ============================================================
// PRIVATE VARIABLES
// ============================================================

// Hardware Serial 2 for CAM communication
HardwareSerial CamSerial(2);

// ============================================================
// DEBUG LOGGING
// ============================================================

#if defined(DEBUG_ENABLED) && defined(DEBUG_SERIAL_COMM)
    #define LOG_SERIAL(msg) Serial.print("[SERIAL] "); Serial.println(msg)
    #define LOG_SERIAL_VAL(msg, val) Serial.print("[SERIAL] "); Serial.print(msg); Serial.println(val)
#else
    #define LOG_SERIAL(msg)
    #define LOG_SERIAL_VAL(msg, val)
#endif

// ============================================================
// PRIVATE FUNCTIONS
// ============================================================

static CamResponseData parseResponse(String& data) {
    CamResponseData response;
    response.type = RESPONSE_NONE;
    response.binNumber = 0;
    response.errorMessage = "";
    
    data.trim();
    
    if (data.length() == 0) {
        return response;
    }
    
    LOG_SERIAL_VAL("RX: ", data);
    
    // Check for BIN response: "BIN:1", "BIN:2", "BIN:3"
    if (data.startsWith(RESP_BIN_PREFIX)) {
        response.type = RESPONSE_BIN;
        response.binNumber = data.substring(strlen(RESP_BIN_PREFIX)).toInt();
        LOG_SERIAL_VAL("Parsed bin number: ", response.binNumber);
        return response;
    }
    
    // Check for ERROR response: "ERROR:message"
    if (data.startsWith(RESP_ERROR_PREFIX)) {
        response.type = RESPONSE_ERROR;
        response.errorMessage = data.substring(strlen(RESP_ERROR_PREFIX));
        LOG_SERIAL_VAL("Parsed error: ", response.errorMessage);
        return response;
    }
    
    // Check for PONG response
    if (data == RESP_PONG) {
        response.type = RESPONSE_PONG;
        LOG_SERIAL("Parsed PONG");
        return response;
    }
    
    // Unknown response
    LOG_SERIAL_VAL("Unknown response: ", data);
    return response;
}

// ============================================================
// PUBLIC FUNCTIONS
// ============================================================

void serialComm_init() {
    // Initialize Hardware Serial 2
    // RX = PIN_SERIAL_RX (GPIO16)
    // TX = PIN_SERIAL_TX (GPIO17)
    CamSerial.begin(SERIAL_BAUD, SERIAL_8N1, PIN_SERIAL_RX, PIN_SERIAL_TX);
    
    // Set timeout for readStringUntil
    CamSerial.setTimeout(SERIAL_READ_TIMEOUT_MS);
    
    LOG_SERIAL("Initialized");
    LOG_SERIAL_VAL("Baud rate: ", SERIAL_BAUD);
    LOG_SERIAL_VAL("RX Pin: ", PIN_SERIAL_RX);
    LOG_SERIAL_VAL("TX Pin: ", PIN_SERIAL_TX);
}

void serialComm_sendCapture() {
    LOG_SERIAL("TX: CAPTURE");
    CamSerial.println(CMD_CAPTURE);
}

void serialComm_sendPing() {
    LOG_SERIAL("TX: PING");
    CamSerial.println(CMD_PING);
}

CamResponseData serialComm_checkResponse() {
    CamResponseData response;
    response.type = RESPONSE_NONE;
    response.binNumber = 0;
    response.errorMessage = "";
    
    if (!CamSerial.available()) {
        return response;
    }
    
    String data = CamSerial.readStringUntil(MSG_TERMINATOR);
    return parseResponse(data);
}

CamResponseData serialComm_waitResponse(unsigned long timeoutMs) {
    CamResponseData response;
    response.type = RESPONSE_NONE;
    response.binNumber = 0;
    response.errorMessage = "";
    
    unsigned long startTime = millis();
    
    LOG_SERIAL_VAL("Waiting for response, timeout: ", timeoutMs);
    
    while (millis() - startTime < timeoutMs) {
        if (CamSerial.available()) {
            String data = CamSerial.readStringUntil(MSG_TERMINATOR);
            response = parseResponse(data);
            
            if (response.type != RESPONSE_NONE) {
                return response;
            }
        }
        
        // Yield to other tasks
        delay(10);
    }
    
    // Timeout
    LOG_SERIAL("Response timeout!");
    response.type = RESPONSE_TIMEOUT;
    return response;
}

void serialComm_clearBuffer() {
    while (CamSerial.available()) {
        CamSerial.read();
    }
    LOG_SERIAL("Buffer cleared");
}

bool serialComm_available() {
    return CamSerial.available() > 0;
}
