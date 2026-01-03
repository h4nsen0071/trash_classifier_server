/**
 * @file camera_pins.h
 * @brief Pin definitions for ESP32-CAM (AI-Thinker)
 * 
 * Đây là cấu hình chuẩn cho AI-Thinker ESP32-CAM
 * KHÔNG CẦN CHỈNH SỬA trừ khi dùng board khác
 */

#ifndef CAMERA_PINS_H
#define CAMERA_PINS_H

// ============================================================
// AI-THINKER ESP32-CAM PINS (OV2640)
// ============================================================

// Power/Reset
#define PWDN_GPIO_NUM       32      // Power down
#define RESET_GPIO_NUM      -1      // No reset pin

// Clock
#define XCLK_GPIO_NUM       0       // External clock

// I2C (SCCB)
#define SIOD_GPIO_NUM       26      // I2C SDA
#define SIOC_GPIO_NUM       27      // I2C SCL

// Data pins (D0-D7)
#define Y2_GPIO_NUM         5       // D0
#define Y3_GPIO_NUM         18      // D1
#define Y4_GPIO_NUM         19      // D2
#define Y5_GPIO_NUM         21      // D3
#define Y6_GPIO_NUM         36      // D4
#define Y7_GPIO_NUM         39      // D5
#define Y8_GPIO_NUM         34      // D6
#define Y9_GPIO_NUM         35      // D7

// Sync signals
#define VSYNC_GPIO_NUM      25      // Vertical sync
#define HREF_GPIO_NUM       23      // Horizontal reference
#define PCLK_GPIO_NUM       22      // Pixel clock

// ============================================================
// FLASH LED
// ============================================================

#define FLASH_GPIO_NUM      4       // Onboard flash LED

// ============================================================
// SERIAL PINS (Built-in UART0)
// ============================================================

#define PIN_TX              1       // U0TXD - Gửi đến Controller
#define PIN_RX              3       // U0RXD - Nhận từ Controller

#endif // CAMERA_PINS_H
