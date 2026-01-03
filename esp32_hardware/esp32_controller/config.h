/**
 * @file config.h
 * @brief Cấu hình ESP32 Controller
 */

#ifndef CONFIG_H
#define CONFIG_H

// === DISTANCE SENSOR ===
#define OBJECT_DISTANCE_CM      20      // Khoảng cách phát hiện (cm)
#define OBJECT_CLEAR_CM         30      // Khoảng cách clear (cm)
#define STABILITY_CHECKS        3       // Số lần check ổn định
#define STABILITY_DELAY_MS      200
#define MAX_DISTANCE_CM         400

// === SERVO ===
#define SERVO_OPEN_ANGLE        90
#define SERVO_CLOSE_ANGLE       0
#define SERVO_MOVE_DELAY_MS     500
#define LID_OPEN_DURATION_MS    5000    // Thời gian giữ nắp mở

// === TIMING ===
#define SENSOR_READ_INTERVAL_MS 100
#define CAM_RESPONSE_TIMEOUT_MS 30000   // Timeout chờ CAM
#define COOLDOWN_DURATION_MS    3000

// === LED ===
#define LED_BLINK_FAST_MS       200
#define LED_BLINK_SLOW_MS       1000
#define LED_ERROR_BLINK_COUNT   5

// === SERIAL ===
#define SERIAL_BAUD             115200
#define SERIAL_READ_TIMEOUT_MS  100

// === DEBUG (comment để tắt) ===
#define DEBUG_ENABLED

// Debug từng module (cần DEBUG_ENABLED)
#define DEBUG_DISTANCE
#define DEBUG_SERVO
#define DEBUG_STATE
#define DEBUG_SERIAL_COMM
#define DEBUG_LED
#define DEBUG_LCD

// === SYSTEM INFO ===
#define FIRMWARE_VERSION        "1.0.0"

#endif // CONFIG_H
