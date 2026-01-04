/**
 * @file lcd_display.h
 * @brief LCD 16x2 I2C Display Module
 */

#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>

/**
 * Khởi tạo LCD
 * @return true nếu thành công
 */
bool lcdDisplay_init();

/**
 * Xóa màn hình
 */
void lcdDisplay_clear();

/**
 * Hiển thị text trên dòng 1 (trên)
 * @param text Nội dung (max 16 ký tự)
 */
void lcdDisplay_line1(const char* text);

/**
 * Hiển thị text trên dòng 2 (dưới)
 * @param text Nội dung (max 16 ký tự)
 */
void lcdDisplay_line2(const char* text);

/**
 * Hiển thị text trên cả 2 dòng
 * @param line1 Dòng 1
 * @param line2 Dòng 2
 */
void lcdDisplay_show(const char* line1, const char* line2);

/**
 * Hiển thị trạng thái IDLE (chờ rác)
 */
void lcdDisplay_showIdle();

/**
 * Hiển thị trạng thái đang xử lý
 */
void lcdDisplay_showProcessing();

/**
 * Hiển thị kết quả phân loại
 * @param binName Tên thùng (VD: "Thùng Đỏ")
 * @param wasteType Loại rác (VD: "Paper")
 */
void lcdDisplay_showResult(const char* binName, const char* wasteType);

/**
 * Hiển thị lỗi
 * @param errorMsg Thông báo lỗi
 */
void lcdDisplay_showError(const char* errorMsg);

/**
 * Hiển thị thông tin khởi động
 */
void lcdDisplay_showStartup();

/**
 * Bật/tắt backlight
 * @param on true = bật, false = tắt
 */
void lcdDisplay_backlight(bool on);

#endif // LCD_DISPLAY_H
