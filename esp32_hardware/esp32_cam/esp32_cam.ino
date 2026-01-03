/**
 * @file esp32_cam.ino
 * @brief Smart Bin Camera - Main Entry Point
 * 
 * ESP32-CAM quản lý:
 * - Kết nối WiFi
 * - Chụp ảnh với camera OV2640
 * - Gửi ảnh lên server để phân loại
 * - Giao tiếp Serial với ESP32 Controller
 * 
 * @author SmartBin Team
 * @version 1.0.0
 */

// ============================================================
// INCLUDES
// ============================================================

#include "config.h"
#include "camera_pins.h"
#include "wifi_manager.h"
#include "camera_handler.h"
#include "http_client.h"
#include "serial_comm.h"
#include "protocol.h"

// ============================================================
// GLOBAL VARIABLES
// ============================================================

static bool systemReady = false;

// ============================================================
// COMMAND HANDLERS
// ============================================================

void handleCaptureCommand() {
    // Log để debug (sẽ đi ra Serial - Controller có thể ignore)
    // Trong production, bỏ các Serial.print này
    
    #ifdef DEBUG_ENABLED
        // Gửi debug info qua Serial2 nếu có
        // Hoặc bỏ qua để không làm nhiễu serial chính
    #endif
    
    // Kiểm tra WiFi
    if (!wifiManager_isConnected()) {
        serialComm_sendError(ERR_WIFI_DISCONNECTED);
        return;
    }
    
    // Chụp ảnh và encode base64
    String imageBase64;
    if (!cameraHandler_captureToBase64(imageBase64)) {
        serialComm_sendError(ERR_CAPTURE_FAILED);
        return;
    }
    
    // Gửi lên server
    ClassificationResult result = httpClient_classify(imageBase64);
    
    // Giải phóng memory
    imageBase64 = "";
    
    // Xử lý kết quả
    if (result.success) {
        serialComm_sendBin(result.binNumber);
    } else {
        // Map error to error code
        if (result.error.indexOf("WiFi") >= 0) {
            serialComm_sendError(ERR_WIFI_DISCONNECTED);
        } else if (result.error.indexOf("Connection") >= 0 || result.error.indexOf("timeout") >= 0) {
            serialComm_sendError(ERR_SERVER_TIMEOUT);
        } else if (result.error.indexOf("confidence") >= 0 || result.error.indexOf("Confidence") >= 0) {
            serialComm_sendError(ERR_LOW_CONFIDENCE);
        } else if (result.error.indexOf("HTTP") >= 0) {
            serialComm_sendError(ERR_SERVER_ERROR);
        } else {
            serialComm_sendError(ERR_INVALID_RESPONSE);
        }
    }
}

void handlePingCommand() {
    serialComm_sendPong();
}

void handleStatusCommand() {
    if (systemReady && wifiManager_isConnected()) {
        serialComm_sendReady();
    } else if (!wifiManager_isConnected()) {
        serialComm_sendError(ERR_WIFI_DISCONNECTED);
    } else {
        serialComm_sendError(ERR_CAMERA_INIT_FAILED);
    }
}

// ============================================================
// SETUP
// ============================================================

void setup() {
    // Khởi tạo Serial cho giao tiếp với Controller
    // LƯU Ý: Serial này cũng được dùng để debug khi chưa nối Controller
    serialComm_init();
    
    delay(1000);
    
    // Startup message (sẽ thấy trên Serial Monitor khi chưa nối Controller)
    Serial.println();
    Serial.println("========================================");
    Serial.println("    Smart Bin Camera v" FIRMWARE_VERSION);
    Serial.println("========================================");
    Serial.println();
    
    // Khởi tạo Camera
    Serial.print("[INIT] Camera... ");
    if (!cameraHandler_init()) {
        Serial.println("FAILED!");
        Serial.println("[ERROR] Camera init failed. Check connections.");
        // Không return, tiếp tục để có thể debug
    } else {
        Serial.println("OK");
    }
    
    // Khởi tạo WiFi
    Serial.print("[INIT] WiFi... ");
    if (!wifiManager_init()) {
        Serial.println("FAILED!");
        Serial.println("[WARNING] WiFi not connected. Will retry...");
    } else {
        Serial.println("OK");
        Serial.print("[INIT] IP: ");
        Serial.println(wifiManager_getIP());
    }
    
    // Khởi tạo HTTP client
    Serial.print("[INIT] HTTP client... ");
    httpClient_init();
    Serial.println("OK");
    
    // Check system status
    systemReady = wifiManager_isConnected();
    
    Serial.println();
    if (systemReady) {
        Serial.println("[INIT] System ready!");
        Serial.println("[INIT] Waiting for commands from Controller...");
    } else {
        Serial.println("[INIT] System partially ready.");
        Serial.println("[INIT] Some features may not work.");
    }
    Serial.println();
    
    // Thông báo ready
    if (systemReady) {
        serialComm_sendReady();
    }
}

// ============================================================
// MAIN LOOP
// ============================================================

void loop() {
    // Duy trì WiFi connection
    wifiManager_maintain();
    
    // Update system status
    systemReady = wifiManager_isConnected();
    
    // Kiểm tra commands từ Controller
    ControllerCommand cmd = serialComm_checkCommand();
    
    switch (cmd) {
        case CMD_CAPTURE_REQ:
            handleCaptureCommand();
            break;
            
        case CMD_PING_REQ:
            handlePingCommand();
            break;
            
        case CMD_STATUS_REQ:
            handleStatusCommand();
            break;
            
        case CMD_NONE:
        default:
            // Không có command, tiếp tục loop
            break;
    }
    
    // Yield để tránh watchdog reset
    delay(10);
}
