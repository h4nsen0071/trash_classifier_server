/**
 * @file protocol.h
 * @brief Shared protocol definitions between ESP32-CAM and ESP32 Controller
 * 
 * ĐÂY LÀ FILE DÙNG CHUNG - COPY VÀO CẢ 2 PROJECTS
 * Định nghĩa giao thức Serial communication
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

// ============================================================
// SERIAL PROTOCOL VERSION
// ============================================================
#define PROTOCOL_VERSION "1.0"

// ============================================================
// BAUD RATE (PHẢI GIỐNG NHAU Ở CẢ 2 BOARDS)
// ============================================================
#define SERIAL_BAUD_RATE 115200

// ============================================================
// COMMANDS (Controller → CAM)
// ============================================================
#define CMD_CAPTURE     "CAPTURE"      // Lệnh chụp ảnh và phân loại
#define CMD_PING        "PING"         // Kiểm tra kết nối
#define CMD_STATUS      "STATUS"       // Yêu cầu trạng thái CAM

// ============================================================
// RESPONSES (CAM → Controller)
// ============================================================

// Success responses
#define RESP_BIN_PREFIX "BIN:"         // Tiền tố kết quả: "BIN:1", "BIN:2", "BIN:3"
#define RESP_PONG       "PONG"         // Phản hồi PING
#define RESP_READY      "READY"        // CAM sẵn sàng

// Error responses  
#define RESP_ERROR_PREFIX "ERROR:"     // Tiền tố lỗi

// Error codes
#define ERR_WIFI_DISCONNECTED   "WIFI_DISCONNECTED"
#define ERR_CAMERA_INIT_FAILED  "CAMERA_INIT_FAILED"
#define ERR_CAPTURE_FAILED      "CAPTURE_FAILED"
#define ERR_SERVER_TIMEOUT      "SERVER_TIMEOUT"
#define ERR_SERVER_ERROR        "SERVER_ERROR"
#define ERR_LOW_CONFIDENCE      "LOW_CONFIDENCE"
#define ERR_INVALID_RESPONSE    "INVALID_RESPONSE"
#define ERR_UNKNOWN_CLASS       "UNKNOWN_CLASS"

// ============================================================
// BIN MAPPING (Server class → Bin number)
// ============================================================
#define BIN_PAPER       1   // Giấy
#define BIN_PLASTIC     2   // Nhựa  
#define BIN_GLASS       3   // Thủy tinh

// Class names từ server
#define CLASS_PAPER     "paper"
#define CLASS_PLASTIC   "plastic"
#define CLASS_GLASS     "glass"

// ============================================================
// TIMEOUTS
// ============================================================
#define SERIAL_READ_TIMEOUT_MS  100     // Timeout đọc serial
#define CAPTURE_TIMEOUT_MS      30000   // Timeout chờ kết quả (30s)

// ============================================================
// MESSAGE TERMINATORS
// ============================================================
#define MSG_TERMINATOR  '\n'    // Kết thúc message bằng newline

#endif // PROTOCOL_H
