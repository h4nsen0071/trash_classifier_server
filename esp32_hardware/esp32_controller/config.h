/**
 * @file config.h
 * @brief CẤU HÌNH CHÍNH CHO ESP32 CONTROLLER
 * 
 * ⚙️ CHỈNH SỬA CÁC GIÁ TRỊ Ở ĐÂY ĐỂ THAY ĐỔI HÀNH VI HỆ THỐNG
 * Không cần sửa các file khác!
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// 🎯 DISTANCE SENSOR - Cảm biến khoảng cách
// ============================================================

// Khoảng cách phát hiện vật thể (cm)
// Giảm nếu thùng rác nhỏ, tăng nếu thùng rác lớn
#define OBJECT_DISTANCE_CM      20

// Khoảng cách "an toàn" - không còn vật thể (cm)
// Nên lớn hơn OBJECT_DISTANCE_CM khoảng 10cm
#define OBJECT_CLEAR_CM         30

// Số lần check liên tục để xác nhận vật thể ổn định
// Tăng lên nếu có nhiều nhiễu (false positive)
#define STABILITY_CHECKS        3

// Thời gian giữa các lần check (ms)
#define STABILITY_DELAY_MS      200

// Khoảng cách tối đa sensor có thể đọc (cm)
#define MAX_DISTANCE_CM         400

// ============================================================
// 🔧 SERVO - Điều khiển nắp thùng
// ============================================================

// Góc mở nắp (độ) - Điều chỉnh tùy theo cơ cấu thùng
#define SERVO_OPEN_ANGLE        90

// Góc đóng nắp (độ)
#define SERVO_CLOSE_ANGLE       0

// Thời gian chờ servo di chuyển xong (ms)
#define SERVO_MOVE_DELAY_MS     500

// Thời gian giữ nắp mở để user thả rác (ms)
// 5 giây là đủ cho hầu hết trường hợp
#define LID_OPEN_DURATION_MS    5000

// ============================================================
// ⏱️ TIMING - Các khoảng thời gian
// ============================================================

// Chu kỳ đọc sensor trong IDLE (ms)
#define SENSOR_READ_INTERVAL_MS 100

// Timeout chờ phản hồi từ ESP32-CAM (ms)
// 30 giây để đủ thời gian cho network chậm
#define CAM_RESPONSE_TIMEOUT_MS 30000

// Thời gian cooldown sau khi xử lý xong (ms)
// Tránh trigger lại ngay lập tức
#define COOLDOWN_DURATION_MS    3000

// ============================================================
// 💡 LED - Chỉ báo trạng thái
// ============================================================

// Chu kỳ blink khi đang xử lý (ms)
#define LED_BLINK_FAST_MS       200

// Chu kỳ blink khi có lỗi (ms)
#define LED_BLINK_SLOW_MS       1000

// Số lần blink khi có lỗi
#define LED_ERROR_BLINK_COUNT   5

// ============================================================
// 🔌 SERIAL COMMUNICATION
// ============================================================

// Baud rate (PHẢI GIỐNG ESP32-CAM)
#define SERIAL_BAUD             115200

// ============================================================
// 🐛 DEBUG - Bật/Tắt logging
// ============================================================

// Bật logging chi tiết (comment để tắt)
#define DEBUG_ENABLED

// Log từng module riêng (comment để tắt module không cần)
#define DEBUG_DISTANCE
#define DEBUG_SERVO
#define DEBUG_STATE
#define DEBUG_SERIAL_COMM
#define DEBUG_LED
#define DEBUG_LCD

// ============================================================
// 📊 SYSTEM INFO
// ============================================================

#define FIRMWARE_VERSION        "1.0.0"
#define DEVICE_NAME             "SmartBin-Controller"

#endif // CONFIG_H
