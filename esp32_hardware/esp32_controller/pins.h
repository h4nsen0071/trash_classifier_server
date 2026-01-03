/**
 * @file pins.h
 * @brief Pin definitions for ESP32 Controller
 * 
 * Định nghĩa tất cả GPIO pins sử dụng
 * Sửa ở đây nếu cần thay đổi wiring
 */

#ifndef PINS_H
#define PINS_H

// ============================================================
// 📡 SERIAL COMMUNICATION (to ESP32-CAM)
// ============================================================

// Hardware Serial 2
#define PIN_SERIAL_RX       16      // GPIO16 - Nhận data từ CAM (nối với CAM TX)
#define PIN_SERIAL_TX       17      // GPIO17 - Gửi data đến CAM (nối với CAM RX)

// ============================================================
// 📏 DISTANCE SENSOR (HC-SR04)
// ============================================================

#define PIN_TRIG            18      // GPIO18 - Trigger pulse
#define PIN_ECHO            19      // GPIO19 - Echo receive

// ============================================================
// 🔧 SERVOS (SG90) - Theo màu thùng rác
// ============================================================

#define PIN_SERVO_RED       12      // GPIO12 - Thùng đỏ (Bin 1)
#define PIN_SERVO_GREEN     26      // GPIO26 - Thùng xanh (Bin 2)
#define PIN_SERVO_GRAY      32      // GPIO32 - Thùng xám (Bin 3)

// Alias cho backward compatibility
#define PIN_SERVO_BIN1      PIN_SERVO_RED
#define PIN_SERVO_BIN2      PIN_SERVO_GREEN
#define PIN_SERVO_BIN3      PIN_SERVO_GRAY

// ============================================================
// 📺 LCD I2C (16x2)
// ============================================================

#define PIN_LCD_SDA         21      // GPIO21 - I2C SDA
#define PIN_LCD_SCL         22      // GPIO22 - I2C SCL
#define LCD_ADDRESS         0x27    // Địa chỉ I2C LCD (thử 0x3F nếu không hoạt động)
#define LCD_COLS            16      // Số cột
#define LCD_ROWS            2       // Số hàng

// ============================================================
// 💡 LED STATUS
// ============================================================

#define PIN_LED_STATUS      2       // GPIO2 - Onboard LED

// ============================================================
// 🔊 BUZZER (Optional - cho tương lai)
// ============================================================

// #define PIN_BUZZER       4       // GPIO4 - Buzzer (uncomment nếu dùng)

#endif // PINS_H
