/**
 * @file blynk_integration.h
 * @brief Blynk IoT - Điều khiển servo + Thông báo rác đầy
 */

#ifndef BLYNK_INTEGRATION_H
#define BLYNK_INTEGRATION_H

// ============================================================
// CẤU HÌNH BLYNK - THAY ĐỔI Ở ĐÂY
// ============================================================
#define BLYNK_TEMPLATE_ID   "TMPL63afuGzmK"
#define BLYNK_TEMPLATE_NAME "Thungracthongminh"
#define BLYNK_AUTH_TOKEN    "0D4LK3MQhDcpVVSR4OMC7PdWsCv12Rh3"

#define BLYNK_WIFI_SSID     "iicha"
#define BLYNK_WIFI_PASS     "iicha84b1"

// Virtual Pins cho servo
#define VPIN_SERVO_RED      V1
#define VPIN_SERVO_GREEN    V2
#define VPIN_SERVO_GRAY     V3

// ============================================================
// CẢM BIẾN MỨC RÁC (HC-SR04 cho 3 thùng)
// Tham khảo từ blynk.ino
// ============================================================
#define PIN_TRIG_RED        14      // Trig thùng Đỏ
#define PIN_ECHO_RED        27      // Echo thùng Đỏ
#define PIN_TRIG_GREEN      25      // Trig thùng Xanh
#define PIN_ECHO_GREEN      33      // Echo thùng Xanh
// LƯU Ý: GPIO 34 chỉ là INPUT! Nếu không hoạt động, đổi sang GPIO 13 hoặc 15
#define PIN_TRIG_GRAY       13      // Trig thùng Xám (đổi từ 34)
#define PIN_ECHO_GRAY       35      // Echo thùng Xám

// Ngưỡng rác đầy (cm)
#define TRASH_FULL_CM       4       // <= 4cm = đầy
#define TRASH_RESET_CM      5       // > 5cm = reset thông báo

// ============================================================
// FUNCTIONS
// ============================================================
bool blynk_init();
void blynk_update();

#endif
