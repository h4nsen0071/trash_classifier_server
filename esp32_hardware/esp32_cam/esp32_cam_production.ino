/**
 * @file esp32_cam.ino  
 * @brief Smart Bin Camera - Main Entry Point (PRODUCTION VERSION - NO DEBUG LOG)
 * 
 * ESP32-CAM quản lý:
 * - Kết nối WiFi
 * - Chụp ảnh với camera OV2640
 * - Gửi ảnh lên server để phân loại
 * - Giao tiếp Serial với ESP32 Controller
 * 
 * ⚠️ QUAN TRỌNG: TẤT CẢ DEBUG LOG ĐÃ ĐƯỢC TẮT
 * ESP32-CAM chỉ gửi protocol messages (READY, BIN:X, ERROR:X, PONG)
 * KHÔNG in bất kỳ debug log nào để tránh xung đột với serial communication
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
    if (wifiManager_isConnected()) {
        serialComm_sendReady();
    } else {
        serialComm_sendError(ERR_WIFI_DISCONNECTED);
    }
}

// ============================================================
// SETUP
// ============================================================

void setup() {
    // Khởi tạo Serial communication
    serialComm_init();
    
    // ⚠️ KHÔNG in log khi chạy production (xung đột với serial protocol)
    // Tất cả debug log đã được tắt bằng cách comment DEBUG_ENABLED trong config.h
    
    // Khởi tạo Camera (im lặng - không in log)
    bool cameraOk = cameraHandler_init();
    
    // Khởi tạo WiFi (im lặng - không in log)
    bool wifiOk = wifiManager_init();
    
    // Khởi tạo HTTP client (im lặng - không in log)
    httpClient_init();
    
    // Check system status
    systemReady = wifiManager_isConnected();
    
    // Thông báo ready (PROTOCOL MESSAGE - không phải debug log)
    if (systemReady) {
        serialComm_sendReady();
    }
    
    // System đã sẵn sàng, không in gì cả
    // Chỉ chờ commands từ Controller
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
