/**
 * @file serial_comm.h
 * @brief Serial Communication Module (Controller side)
 */

#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

#include <Arduino.h>

// Response types
enum CamResponse {
    RESPONSE_NONE,          // Chưa có response
    RESPONSE_BIN,           // Nhận được BIN:X
    RESPONSE_ERROR,         // Nhận được ERROR:msg
    RESPONSE_PONG,          // Nhận được PONG
    RESPONSE_TIMEOUT        // Timeout
};

// Response data structure
struct CamResponseData {
    CamResponse type;
    int binNumber;          // Chỉ có giá trị khi type = RESPONSE_BIN
    String errorMessage;    // Chỉ có giá trị khi type = RESPONSE_ERROR
};

/**
 * Khởi tạo Serial communication
 */
void serialComm_init();

/**
 * Gửi lệnh CAPTURE đến ESP32-CAM
 */
void serialComm_sendCapture();

/**
 * Gửi lệnh PING đến ESP32-CAM
 */
void serialComm_sendPing();

/**
 * Kiểm tra và đọc response từ ESP32-CAM (non-blocking)
 * @return Response data structure
 */
CamResponseData serialComm_checkResponse();

/**
 * Chờ response với timeout (blocking)
 * @param timeoutMs Thời gian chờ tối đa (ms)
 * @return Response data structure
 */
CamResponseData serialComm_waitResponse(unsigned long timeoutMs);

/**
 * Xóa buffer serial
 */
void serialComm_clearBuffer();

/**
 * Kiểm tra có data trong buffer không
 * @return true nếu có data
 */
bool serialComm_available();

#endif // SERIAL_COMM_H
