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

// Khoi tao LCD voi dia chi I2C, so cot, so hang
static LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);
static bool initialized = false;

// ============================================================
// DEBUG LOGGING
// ============================================================

#if defined(DEBUG_ENABLED) && defined(DEBUG_LCD)
#define LOG_LCD(msg)        \
    Serial.print("[LCD] "); \
    Serial.println(msg)
#define LOG_LCD_VAL(msg, val) \
    Serial.print("[LCD] ");   \
    Serial.print(msg);        \
    Serial.println(val)
#else
#define LOG_LCD(msg)
#define LOG_LCD_VAL(msg, val)
#endif

// ============================================================
// PRIVATE FUNCTIONS
// ============================================================

static void centerText(const char *text, char *buffer, int width)
{
    int len = strlen(text);
    if (len >= width)
    {
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

bool lcdDisplay_init()
{
    LOG_LCD("Initializing...");

    // Khởi tạo I2C
    Wire.begin(PIN_LCD_SDA, PIN_LCD_SCL);

    // Khoi tao LCD
    lcd.init();
    lcd.backlight();

    initialized = true;

    LOG_LCD("Initialized");
    LOG_LCD_VAL("Address: 0x", LCD_ADDRESS);
    LOG_LCD_VAL("Size: ", String(LCD_COLS) + "x" + String(LCD_ROWS));

    return true;
}

void lcdDisplay_clear()
{
    if (!initialized)
        return;
    lcd.clear();
}

void lcdDisplay_line1(const char *text)
{
    if (!initialized)
        return;

    // Tạo buffer 16 ký tự với padding spaces (không cần clear riêng)
    char buffer[LCD_COLS + 1];
    memset(buffer, ' ', LCD_COLS);
    buffer[LCD_COLS] = '\0';

    int len = strlen(text);
    if (len > LCD_COLS)
        len = LCD_COLS;
    memcpy(buffer, text, len);

    lcd.setCursor(0, 0);
    lcd.print(buffer); // Ghi đè 1 lần, không flickering

    LOG_LCD_VAL("Line1: ", text);
}

void lcdDisplay_line2(const char *text)
{
    if (!initialized)
        return;

    // Tạo buffer 16 ký tự với padding spaces
    char buffer[LCD_COLS + 1];
    memset(buffer, ' ', LCD_COLS);
    buffer[LCD_COLS] = '\0';

    int len = strlen(text);
    if (len > LCD_COLS)
        len = LCD_COLS;
    memcpy(buffer, text, len);

    lcd.setCursor(0, 1);
    lcd.print(buffer); // Ghi đè 1 lần, không flickering

    LOG_LCD_VAL("Line2: ", text);
}

void lcdDisplay_show(const char *line1, const char *line2)
{
    if (!initialized)
        return;

    // Không dùng clear(), ghi đè trực tiếp để tránh flickering
    lcdDisplay_line1(line1);
    lcdDisplay_line2(line2);
}

void lcdDisplay_showIdle()
{
    if (!initialized)
        return;

    lcd.setCursor(0, 0);
    lcd.print("[*] Smart Bin   ");
    lcd.setCursor(0, 1);
    lcd.print("Ready...        ");

    LOG_LCD("Showing IDLE");
}

void lcdDisplay_showProcessing()
{
    if (!initialized)
        return;

    lcd.setCursor(0, 0);
    lcd.print("Classifying...  ");
    lcd.setCursor(0, 1);
    lcd.print("Please wait...  ");

    LOG_LCD("Showing PROCESSING");
}

void lcdDisplay_showResult(const char *binName, const char *wasteType)
{
    if (!initialized)
        return;

    // Line 1: wasteType (padding to 16 chars)
    char line1[LCD_COLS + 1];
    memset(line1, ' ', LCD_COLS);
    line1[LCD_COLS] = '\0';
    int len = strlen(wasteType);
    if (len > LCD_COLS)
        len = LCD_COLS;
    memcpy(line1, wasteType, len);

    lcd.setCursor(0, 0);
    lcd.print(line1);

    // Line 2: "-> " + binName (padding to 16 chars)
    char line2[LCD_COLS + 1];
    memset(line2, ' ', LCD_COLS);
    line2[LCD_COLS] = '\0';
    line2[0] = '-';
    line2[1] = '>';
    line2[2] = ' ';
    len = strlen(binName);
    if (len > LCD_COLS - 3)
        len = LCD_COLS - 3;
    memcpy(line2 + 3, binName, len);

    lcd.setCursor(0, 1);
    lcd.print(line2);

    LOG_LCD_VAL("Result: ", String(wasteType) + " -> " + String(binName));
}

void lcdDisplay_showError(const char *errorMsg)
{
    if (!initialized)
        return;

    // Line 1: "[X] Error!" (padding)
    lcd.setCursor(0, 0);
    lcd.print("[X] Error!      ");

    // Line 2: errorMsg (padding to 16 chars)
    char line2[LCD_COLS + 1];
    memset(line2, ' ', LCD_COLS);
    line2[LCD_COLS] = '\0';
    int len = strlen(errorMsg);
    if (len > LCD_COLS)
        len = LCD_COLS;
    memcpy(line2, errorMsg, len);

    lcd.setCursor(0, 1);
    lcd.print(line2);

    LOG_LCD_VAL("Error: ", errorMsg);
}

void lcdDisplay_showStartup()
{
    if (!initialized)
        return;

    lcd.setCursor(0, 0);
    lcd.print("Smart Trash Bin ");
    lcd.setCursor(0, 1);
    lcd.print("Starting...     ");

    LOG_LCD("Showing STARTUP");
}

void lcdDisplay_backlight(bool on)
{
    if (!initialized)
        return;

    if (on)
    {
        lcd.backlight();
    }
    else
    {
        lcd.noBacklight();
    }

    LOG_LCD_VAL("Backlight: ", on ? "ON" : "OFF");
}
