/**
 * @file lcd_display.cpp
 * @brief LCD 16x2 I2C Display Implementation
 * 
 * Sử dụng thư viện LiquidCrystal_I2C
 * Cài đặt: Library Manager → tìm "LiquidCrystal I2C" by Frank de Brabander
 */

#include "lcd_display.h"
#include "config.h"
#include "pins.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ============================================================
// PRIVATE VARIABLES
// ============================================================

// Khởi tạo LCD với địa chỉ I2C, số cột, số hàng
static LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);
static bool initialized = false;

// Custom characters (icons)
// Trash icon
static byte trashIcon[8] = {
    0b01110,
    0b11111,
    0b10001,
    0b10101,
    0b10101,
    0b10001,
    0b11111,
    0b00000
};

// Check icon
static byte checkIcon[8] = {
    0b00000,
    0b00001,
    0b00011,
    0b10110,
    0b11100,
    0b01000,
    0b00000,
    0b00000
};

// Error icon
static byte errorIcon[8] = {
    0b00000,
    0b10001,
    0b01010,
    0b00100,
    0b01010,
    0b10001,
    0b00000,
    0b00000
};

// ============================================================
// DEBUG LOGGING
// ============================================================

#if defined(DEBUG_ENABLED) && defined(DEBUG_LCD)
    #define LOG_LCD(msg) Serial.print("[LCD] "); Serial.println(msg)
    #define LOG_LCD_VAL(msg, val) Serial.print("[LCD] "); Serial.print(msg); Serial.println(val)
#else
    #define LOG_LCD(msg)
    #define LOG_LCD_VAL(msg, val)
#endif

// ============================================================
// PRIVATE FUNCTIONS
// ============================================================

static void centerText(const char* text, char* buffer, int width) {
    int len = strlen(text);
    if (len >= width) {
        strncpy(buffer, text, width);
        buffer[width] = '\0';
        return;
    }
    
    int padding = (width - len) / 2;
    memset(buffer, ' ', width);
    strncpy(buffer + padding, text, len);
    buffer[width] = '\0';
}

// ============================================================
// PUBLIC FUNCTIONS
// ============================================================

bool lcdDisplay_init() {
    LOG_LCD("Initializing...");
    
    // Khởi tạo I2C
    Wire.begin(PIN_LCD_SDA, PIN_LCD_SCL);
    
    // Khởi tạo LCD
    lcd.init();
    lcd.backlight();
    
    // Tạo custom characters
    lcd.createChar(0, trashIcon);
    lcd.createChar(1, checkIcon);
    lcd.createChar(2, errorIcon);
    
    initialized = true;
    
    LOG_LCD("Initialized");
    LOG_LCD_VAL("Address: 0x", LCD_ADDRESS);
    LOG_LCD_VAL("Size: ", String(LCD_COLS) + "x" + String(LCD_ROWS));
    
    return true;
}

void lcdDisplay_clear() {
    if (!initialized) return;
    lcd.clear();
}

void lcdDisplay_line1(const char* text) {
    if (!initialized) return;
    
    lcd.setCursor(0, 0);
    lcd.print("                "); // Clear line
    lcd.setCursor(0, 0);
    lcd.print(text);
    
    LOG_LCD_VAL("Line1: ", text);
}

void lcdDisplay_line2(const char* text) {
    if (!initialized) return;
    
    lcd.setCursor(0, 1);
    lcd.print("                "); // Clear line
    lcd.setCursor(0, 1);
    lcd.print(text);
    
    LOG_LCD_VAL("Line2: ", text);
}

void lcdDisplay_show(const char* line1, const char* line2) {
    if (!initialized) return;
    
    lcdDisplay_clear();
    lcdDisplay_line1(line1);
    lcdDisplay_line2(line2);
}

void lcdDisplay_showIdle() {
    if (!initialized) return;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(byte(0)); // Trash icon
    lcd.print(" Smart Bin");
    lcd.setCursor(0, 1);
    lcd.print("Cho rac vao...");
    
    LOG_LCD("Showing IDLE");
}

void lcdDisplay_showProcessing() {
    if (!initialized) return;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dang phan loai");
    lcd.setCursor(0, 1);
    lcd.print("Vui long cho...");
    
    LOG_LCD("Showing PROCESSING");
}

void lcdDisplay_showResult(const char* binName, const char* wasteType) {
    if (!initialized) return;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(byte(1)); // Check icon
    lcd.print(" ");
    lcd.print(wasteType);
    lcd.setCursor(0, 1);
    lcd.print("-> ");
    lcd.print(binName);
    
    LOG_LCD_VAL("Result: ", String(wasteType) + " -> " + String(binName));
}

void lcdDisplay_showError(const char* errorMsg) {
    if (!initialized) return;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(byte(2)); // Error icon
    lcd.print(" Loi!");
    lcd.setCursor(0, 1);
    lcd.print(errorMsg);
    
    LOG_LCD_VAL("Error: ", errorMsg);
}

void lcdDisplay_showStartup() {
    if (!initialized) return;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Smart Trash Bin");
    lcd.setCursor(0, 1);
    lcd.print("Khoi dong...");
    
    LOG_LCD("Showing STARTUP");
}

void lcdDisplay_backlight(bool on) {
    if (!initialized) return;
    
    if (on) {
        lcd.backlight();
    } else {
        lcd.noBacklight();
    }
    
    LOG_LCD_VAL("Backlight: ", on ? "ON" : "OFF");
}
