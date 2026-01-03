/**
 * @file serial_comm.h
 * @brief Serial Communication Module (CAM side)
 */

#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

#include <Arduino.h>

// Command types received from Controller
enum ControllerCommand {
    CMD_NONE,           // Không có command
    CMD_CAPTURE_REQ,    // Yêu cầu chụp ảnh
    CMD_PING_REQ,       // Yêu cầu ping
    CMD_STATUS_REQ      // Yêu cầu status
};

/**
 * Khởi tạo Serial communication
 * LƯU Ý: ESP32-CAM dùng UART0 (Serial) để giao tiếp với Controller
 *        nên không thể dùng Serial để debug khi đang chạy
 */
void serialComm_init();

/**
 * Kiểm tra và đọc command từ Controller (non-blocking)
 * @return ControllerCommand
 */
ControllerCommand serialComm_checkCommand();

/**
 * Gửi kết quả bin về Controller
 * @param binNumber Số bin (1-3)
 */
void serialComm_sendBin(int binNumber);

/**
 * Gửi lỗi về Controller
 * @param errorCode Mã lỗi (xem protocol.h)
 */
void serialComm_sendError(const char* errorCode);

/**
 * Gửi PONG response
 */
void serialComm_sendPong();

/**
 * Gửi trạng thái READY
 */
void serialComm_sendReady();

#endif // SERIAL_COMM_H
