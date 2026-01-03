/**
 * @file servo_controller.h
 * @brief Servo Motor Control Module
 */

#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include <Arduino.h>

// Bin identifiers (theo màu thùng)
#define BIN_RED         1   // Thùng đỏ
#define BIN_GREEN       2   // Thùng xanh
#define BIN_GRAY        3   // Thùng xám

// Alias cho class names từ server (cần map trong config)
// Mặc định: paper→red, plastic→green, glass→gray
#define BIN_PAPER       BIN_RED
#define BIN_PLASTIC     BIN_GREEN
#define BIN_GLASS       BIN_GRAY

/**
 * Khởi tạo tất cả servos
 */
void servoController_init();

/**
 * Mở nắp thùng rác
 * @param binNumber Số thùng (1=Red, 2=Green, 3=Gray)
 * @return true nếu thành công
 */
bool servoController_openBin(int binNumber);

/**
 * Đóng nắp thùng rác
 * @param binNumber Số thùng (1=Red, 2=Green, 3=Gray)
 * @return true nếu thành công
 */
bool servoController_closeBin(int binNumber);

/**
 * Đóng tất cả các thùng
 */
void servoController_closeAll();

/**
 * Kiểm tra bin number có hợp lệ không
 * @param binNumber Số thùng
 * @return true nếu hợp lệ (1-3)
 */
bool servoController_isValidBin(int binNumber);

/**
 * Lấy tên thùng (màu)
 * @param binNumber Số thùng
 * @return Tên thùng ("Đỏ", "Xanh", "Xám")
 */
const char* servoController_getBinName(int binNumber);

/**
 * Lấy tên loại rác
 * @param binNumber Số thùng
 * @return Tên loại rác ("Paper", "Plastic", "Glass")
 */
const char* servoController_getWasteType(int binNumber);

#endif // SERVO_CONTROLLER_H
