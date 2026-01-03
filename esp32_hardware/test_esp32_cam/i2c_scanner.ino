/**
 * @file i2c_scanner.ino
 * @brief Quét I2C để tìm camera address
 * 
 * Upload file này trước để kiểm tra:
 * 1. Camera có kết nối đúng không
 * 2. Address I2C là bao nhiêu
 * 3. SCCB pins có đúng không
 */

#include <Wire.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// AI-Thinker ESP32-CAM SCCB pins
#define SIOD_GPIO  26  // I2C SDA
#define SIOC_GPIO  27  // I2C SCL
#define PWDN_GPIO  32  // Camera power down

// Camera thường có address: 0x21, 0x30, 0x3C
// OV2640: 0x30 hoặc 0x21
// OV5640: 0x3C

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n================================");
    Serial.println("   I2C SCANNER FOR ESP32-CAM");
    Serial.println("================================\n");
    
    // Power on camera
    Serial.println("[1] Powering on camera...");
    pinMode(PWDN_GPIO, OUTPUT);
    digitalWrite(PWDN_GPIO, LOW);  // LOW = power on
    delay(500);
    
    // Init I2C on SCCB pins
    Serial.printf("[2] Init I2C on SDA=%d, SCL=%d\n", SIOD_GPIO, SIOC_GPIO);
    Wire.begin(SIOD_GPIO, SIOC_GPIO);
    Wire.setClock(100000);  // 100kHz for safety
    delay(100);
    
    // Scan I2C
    Serial.println("\n[3] Scanning I2C bus...\n");
    
    int found = 0;
    for (byte addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        byte error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.printf("  ✅ Found device at 0x%02X", addr);
            
            // Identify common devices
            if (addr == 0x21) Serial.print(" (OV2640 alternate)");
            else if (addr == 0x30) Serial.print(" (OV2640 default)");
            else if (addr == 0x3C) Serial.print(" (OV5640 / OLED)");
            else if (addr == 0x27 || addr == 0x3F) Serial.print(" (LCD I2C)");
            else if (addr == 0x68 || addr == 0x69) Serial.print(" (MPU6050)");
            
            Serial.println();
            found++;
        } else if (error == 4) {
            Serial.printf("  ❌ Error at 0x%02X\n", addr);
        }
    }
    
    Serial.println();
    Serial.printf("[RESULT] Found %d device(s)\n\n", found);
    
    if (found == 0) {
        Serial.println("========================================");
        Serial.println("⚠️  NO I2C DEVICES FOUND!");
        Serial.println("========================================");
        Serial.println();
        Serial.println("Possible causes:");
        Serial.println("1. Flex cable disconnected or loose");
        Serial.println("2. Flex cable inserted wrong way");
        Serial.println("   → Contacts should face the BOARD");
        Serial.println("3. Camera module is dead");
        Serial.println("4. Wrong SCCB pins for your board");
        Serial.println("5. Insufficient power (use 5V/2A)");
        Serial.println();
        Serial.println("Try:");
        Serial.println("- Reseat the flex cable firmly");
        Serial.println("- Power cycle the board");
        Serial.println("- Use a different USB cable/power");
    } else {
        Serial.println("========================================");
        Serial.println("✅ I2C device(s) found!");
        Serial.println("========================================");
        Serial.println();
        Serial.println("If OV2640 found (0x30 or 0x21):");
        Serial.println("→ Camera hardware is OK");
        Serial.println("→ Problem may be in camera driver config");
        Serial.println();
        Serial.println("If NOT found:");
        Serial.println("→ Check flex cable connection");
    }
    
    Serial.println("\n[4] Testing camera power cycle...");
    
    // Try power cycle
    digitalWrite(PWDN_GPIO, HIGH);  // Power off
    delay(300);
    digitalWrite(PWDN_GPIO, LOW);   // Power on
    delay(500);
    
    // Scan again
    Serial.println("[5] Scanning again after power cycle...\n");
    found = 0;
    for (byte addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("  ✅ 0x%02X\n", addr);
            found++;
        }
    }
    Serial.printf("\n[RESULT] Found %d device(s) after power cycle\n", found);
    
    Serial.println("\n================================");
    Serial.println("   SCAN COMPLETE");
    Serial.println("================================");
}

void loop() {
    // Nothing - just scan once
    delay(10000);
}
