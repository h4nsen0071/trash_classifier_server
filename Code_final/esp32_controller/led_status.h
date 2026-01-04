/**
 * @file led_status.h
 * @brief LED Status Indicator Module
 */

#ifndef LED_STATUS_H
#define LED_STATUS_H

#include <Arduino.h>

// LED Patterns
enum LedPattern {
    LED_OFF,            // Tắt
    LED_ON,             // Sáng liên tục
    LED_BLINK_FAST,     // Nhấp nháy nhanh (processing)
    LED_BLINK_SLOW,     // Nhấp nháy chậm (error)
    LED_PULSE           // Fade in/out (standby)
};

/**
 * Khởi tạo LED
 */
void ledStatus_init();

/**
 * Đặt pattern cho LED
 * @param pattern Pattern cần hiển thị
 */
void ledStatus_setPattern(LedPattern pattern);

/**
 * Update LED (gọi trong loop)
 * Cần gọi liên tục để blink/pulse hoạt động
 */
void ledStatus_update();

/**
 * Blink LED một số lần (blocking)
 * @param count Số lần blink
 * @param onTime Thời gian sáng (ms)
 * @param offTime Thời gian tắt (ms)
 */
void ledStatus_blinkTimes(int count, int onTime, int offTime);

/**
 * Hiển thị pattern thành công (3 blink nhanh)
 */
void ledStatus_showSuccess();

/**
 * Hiển thị pattern lỗi (5 blink chậm)
 */
void ledStatus_showError();

#endif // LED_STATUS_H
