/**
 * @file espnow_comm.h
 * @brief ESP-NOW Communication Module (Controller side)
 *
 * Thay thế Serial communication bằng ESP-NOW
 * Giao tiếp không dây trực tiếp với ESP32-CAM
 */

#ifndef ESPNOW_COMM_H
#define ESPNOW_COMM_H

#include <Arduino.h>

// Response types (giữ nguyên như serial_comm.h)
enum CamResponse
{
    RESPONSE_NONE,   // Chưa có response
    RESPONSE_BIN,    // Nhận được BIN:X
    RESPONSE_ERROR,  // Nhận được ERROR:msg
    RESPONSE_PONG,   // Nhận được PONG
    RESPONSE_TIMEOUT // Timeout
};

// Response data structure
struct CamResponseData
{
    CamResponse type;
    int binNumber;       // Chỉ có giá trị khi type = RESPONSE_BIN
    String errorMessage; // Chỉ có giá trị khi type = RESPONSE_ERROR
};

/**
 * Khởi tạo ESP-NOW communication
 * @return true nếu thành công
 */
bool espnowComm_init();

/**
 * Gửi lệnh CAPTURE đến ESP32-CAM
 */
void espnowComm_sendCapture();

/**
 * Gửi lệnh PING đến ESP32-CAM
 */
void espnowComm_sendPing();

/**
 * Kiểm tra và đọc response từ ESP32-CAM (non-blocking)
 * @return Response data structure
 */
CamResponseData espnowComm_checkResponse();

/**
 * Chờ response với timeout (blocking)
 * @param timeoutMs Thời gian chờ tối đa (ms)
 * @return Response data structure
 */
CamResponseData espnowComm_waitResponse(unsigned long timeoutMs);

/**
 * Clear response buffer
 */
void espnowComm_clearBuffer();

/**
 * Kiểm tra có response pending không
 */
bool espnowComm_available();

#endif
