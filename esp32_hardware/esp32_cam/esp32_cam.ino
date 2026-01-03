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
    // Kiểm tra WiFi
    if (!wifiManager_isConnected()) {
        serialComm_sendError(ERR_WIFI_DISCONNECTED);
        return;
    }
    
    // Chụp ảnh raw JPEG
    camera_fb_t* fb = cameraHandler_capture();
    if (!fb) {
        serialComm_sendError(ERR_CAPTURE_FAILED);
        return;
    }
    
    // Gửi raw JPEG lên server
    ClassificationResult result = httpClient_classifyRaw(fb->buf, fb->len);
    
    // Giải phóng frame buffer
    cameraHandler_releaseFrame(fb);
    
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
    // ⚠️ Serial KHÔNG được dùng để in log khi chạy với Controller!
    serialComm_init();
    
    delay(500);
    
    #ifdef DEBUG_ENABLED
        // Startup message (CHỈ hiện khi debug - test riêng CAM)
        Serial.println();
        Serial.println("========================================");
        Serial.println("    Smart Bin Camera v" FIRMWARE_VERSION);
        Serial.println("========================================");
        Serial.println();
    #endif
    
    // Khởi tạo Camera
    #ifdef DEBUG_ENABLED
        Serial.print("[INIT] Camera... ");
    #endif
    
    bool cameraOK = cameraHandler_init();
    
    #ifdef DEBUG_ENABLED
        if (!cameraOK) {
            Serial.println("FAILED!");
            Serial.println("[ERROR] Camera init failed. Check connections.");
        } else {
            Serial.println("OK");
        }
    #endif
    
    // ⚠️ Nếu camera lỗi, gửi ERROR ngay
    if (!cameraOK) {
        delay(1000);
        serialComm_sendError(ERR_CAMERA_INIT_FAILED);
        // Không khởi tạo WiFi/HTTP nếu camera lỗi
        systemReady = false;
        return; // Dừng setup(), không tiếp tục
    }
    
    // Khởi tạo WiFi
    #ifdef DEBUG_ENABLED
        Serial.print("[INIT] WiFi... ");
    #endif
    
    bool wifiOK = wifiManager_init();
    
    #ifdef DEBUG_ENABLED
        if (!wifiOK) {
            Serial.println("FAILED!");
            Serial.println("[WARNING] WiFi not connected. Will retry...");
        } else {
            Serial.println("OK");
            Serial.print("[INIT] IP: ");
            Serial.println(wifiManager_getIP());
        }
    #endif
    
    // Khởi tạo HTTP client
    #ifdef DEBUG_ENABLED
        Serial.print("[INIT] HTTP client... ");
    #endif
    
    httpClient_init();
    
    #ifdef DEBUG_ENABLED
        Serial.println("OK");
    #endif
    
    // Check system status
    systemReady = wifiManager_isConnected();
    
    #ifdef DEBUG_ENABLED
        Serial.println();
        if (systemReady) {
            Serial.println("[INIT] System ready!");
            Serial.println("[INIT] Waiting for commands from Controller...");
        } else {
            Serial.println("[INIT] System partially ready.");
            Serial.println("[INIT] Some features may not work.");
        }
        Serial.println();
    #endif
    
    // Thông báo ready cho Controller (PROTOCOL MESSAGE - giữ lại)
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
