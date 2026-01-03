/**
 * @file esp32_controller_simple.ino
 * @brief ESP32 Controller đơn giản - Tất cả trong 1 file
 * 
 * Chức năng:
 * - HC-SR04: Phát hiện vật thể
 * - 3 Servo: Mở/đóng 3 thùng rác
 * - LCD 16x2: Hiển thị trạng thái
 * - Serial: Giao tiếp với ESP32-CAM
 * 
 * ⚠️ Đổi tên thành esp32_controller.ino để sử dụng
 */

#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ============================================================
// CẤU HÌNH - THAY ĐỔI Ở ĐÂY
// ============================================================

// Distance sensor
#define OBJECT_DISTANCE_CM      20    // Khoảng cách phát hiện vật
#define STABILITY_CHECKS        3     // Số lần check ổn định

// Servo angles
#define SERVO_OPEN_ANGLE        90
#define SERVO_CLOSE_ANGLE       0

// Timing
#define LID_OPEN_DURATION_MS    5000  // Thời gian giữ nắp mở
#define CAM_TIMEOUT_MS          30000 // Timeout chờ CAM
#define COOLDOWN_MS             3000  // Cooldown sau xử lý

// Serial baud (phải giống ESP32-CAM)
#define SERIAL_BAUD             115200

// ============================================================
// PINS
// ============================================================

// HC-SR04
#define PIN_TRIG    18
#define PIN_ECHO    19

// Servos
#define PIN_SERVO1  12    // Bin 1 - Đỏ
#define PIN_SERVO2  26    // Bin 2 - Xanh  
#define PIN_SERVO3  32    // Bin 3 - Xám

// Serial to CAM (Hardware Serial 2)
#define PIN_RX2     16
#define PIN_TX2     17

// LED
#define PIN_LED     2

// LCD I2C
#define LCD_ADDR    0x27
#define LCD_COLS    16
#define LCD_ROWS    2

// ============================================================
// GLOBAL OBJECTS
// ============================================================

Servo servo1, servo2, servo3;
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
HardwareSerial SerialCAM(2);

// States
enum State { IDLE, DETECTING, CAPTURING, WAITING, OPENING, HOLDING, CLOSING, COOLDOWN, ERROR_STATE };
State state = IDLE;
unsigned long stateTime = 0;
int stabilityCount = 0;
int currentBin = 0;

// ============================================================
// DISTANCE SENSOR
// ============================================================

float readDistance() {
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);
    
    long duration = pulseIn(PIN_ECHO, HIGH, 30000);
    if (duration == 0) return 999;
    
    return duration * 0.034 / 2;
}

bool objectDetected() {
    float dist = readDistance();
    return (dist > 0 && dist < OBJECT_DISTANCE_CM);
}

// ============================================================
// SERVOS
// ============================================================

void openBin(int bin) {
    switch (bin) {
        case 1: servo1.write(SERVO_OPEN_ANGLE); break;
        case 2: servo2.write(SERVO_OPEN_ANGLE); break;
        case 3: servo3.write(SERVO_OPEN_ANGLE); break;
    }
}

void closeBin(int bin) {
    switch (bin) {
        case 1: servo1.write(SERVO_CLOSE_ANGLE); break;
        case 2: servo2.write(SERVO_CLOSE_ANGLE); break;
        case 3: servo3.write(SERVO_CLOSE_ANGLE); break;
    }
}

void closeAllBins() {
    servo1.write(SERVO_CLOSE_ANGLE);
    servo2.write(SERVO_CLOSE_ANGLE);
    servo3.write(SERVO_CLOSE_ANGLE);
}

const char* getBinName(int bin) {
    switch (bin) {
        case 1: return "Do";      // Đỏ
        case 2: return "Xanh";    // Xanh
        case 3: return "Xam";     // Xám
        default: return "?";
    }
}

// ============================================================
// LCD
// ============================================================

void lcdShowIdle() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Smart Bin Ready");
    lcd.setCursor(0, 1);
    lcd.print("Cho rac vao...");
}

void lcdShowProcessing() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dang xu ly...");
    lcd.setCursor(0, 1);
    lcd.print("Vui long cho");
}

void lcdShowResult(int bin) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Thung: ");
    lcd.print(getBinName(bin));
    lcd.setCursor(0, 1);
    lcd.print("Dang mo nap...");
}

void lcdShowError() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LOI!");
    lcd.setCursor(0, 1);
    lcd.print("Khong phan loai");
}

// ============================================================
// SERIAL COMMUNICATION
// ============================================================

void sendCapture() {
    // Clear buffer
    while (SerialCAM.available()) SerialCAM.read();
    
    SerialCAM.println("CAPTURE");
    Serial.println("[TX] CAPTURE");
}

// Response: 0=timeout, 1-3=bin, -1=error
int waitResponse() {
    unsigned long start = millis();
    String response = "";
    
    while (millis() - start < CAM_TIMEOUT_MS) {
        if (SerialCAM.available()) {
            char c = SerialCAM.read();
            
            if (c == '\n') {
                response.trim();
                
                // Bỏ qua boot messages
                if (response.startsWith("rst:") || 
                    response.startsWith("load:") ||
                    response.startsWith("entry") ||
                    response.startsWith("configsip") ||
                    response.startsWith("clk_drv") ||
                    response.startsWith("mode:") ||
                    response.startsWith("ets") ||
                    response.startsWith("E (") ||
                    response.startsWith("Guru") ||
                    response.startsWith("Core") ||
                    response.startsWith("PC") ||
                    response.startsWith("A") ||
                    response.length() == 0) {
                    response = "";
                    continue;
                }
                
                Serial.print("[RX] ");
                Serial.println(response);
                
                // Parse response
                if (response.startsWith("BIN:")) {
                    int bin = response.substring(4).toInt();
                    if (bin >= 1 && bin <= 3) return bin;
                }
                
                if (response.startsWith("ERROR:")) {
                    return -1;
                }
                
                if (response == "READY" || response == "PONG") {
                    response = "";
                    continue;  // Bỏ qua, chờ BIN
                }
                
                response = "";
            } else {
                response += c;
            }
        }
        delay(1);
    }
    
    return 0;  // Timeout
}

// ============================================================
// STATE MACHINE
// ============================================================

void changeState(State newState) {
    state = newState;
    stateTime = millis();
}

void runStateMachine() {
    switch (state) {
        case IDLE:
            digitalWrite(PIN_LED, LOW);
            if (objectDetected()) {
                stabilityCount = 1;
                changeState(DETECTING);
            }
            break;
            
        case DETECTING:
            digitalWrite(PIN_LED, HIGH);
            delay(200);
            if (objectDetected()) {
                stabilityCount++;
                if (stabilityCount >= STABILITY_CHECKS) {
                    changeState(CAPTURING);
                }
            } else {
                changeState(IDLE);
            }
            break;
            
        case CAPTURING:
            lcdShowProcessing();
            sendCapture();
            changeState(WAITING);
            break;
            
        case WAITING:
            digitalWrite(PIN_LED, (millis() / 200) % 2);  // Blink
            {
                int result = waitResponse();
                if (result >= 1 && result <= 3) {
                    currentBin = result;
                    changeState(OPENING);
                } else {
                    changeState(ERROR_STATE);
                }
            }
            break;
            
        case OPENING:
            digitalWrite(PIN_LED, HIGH);
            lcdShowResult(currentBin);
            Serial.print("[ACTION] Opening bin ");
            Serial.println(currentBin);
            openBin(currentBin);
            changeState(HOLDING);
            break;
            
        case HOLDING:
            if (millis() - stateTime >= LID_OPEN_DURATION_MS) {
                changeState(CLOSING);
            }
            break;
            
        case CLOSING:
            Serial.print("[ACTION] Closing bin ");
            Serial.println(currentBin);
            closeBin(currentBin);
            currentBin = 0;
            changeState(COOLDOWN);
            break;
            
        case COOLDOWN:
            digitalWrite(PIN_LED, LOW);
            if (millis() - stateTime >= COOLDOWN_MS) {
                lcdShowIdle();
                changeState(IDLE);
            }
            break;
            
        case ERROR_STATE:
            lcdShowError();
            Serial.println("[ERROR] Classification failed");
            // Blink LED 5 times
            for (int i = 0; i < 5; i++) {
                digitalWrite(PIN_LED, HIGH);
                delay(200);
                digitalWrite(PIN_LED, LOW);
                delay(200);
            }
            closeAllBins();
            delay(2000);
            lcdShowIdle();
            changeState(IDLE);
            break;
    }
}

// ============================================================
// SETUP
// ============================================================

void setup() {
    // Debug Serial (USB)
    Serial.begin(115200);
    delay(1000);
    
    Serial.println();
    Serial.println("================================");
    Serial.println("  Smart Bin Controller v2.0");
    Serial.println("================================");
    
    // Pins
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    pinMode(PIN_LED, OUTPUT);
    
    // Serial to CAM
    SerialCAM.begin(SERIAL_BAUD, SERIAL_8N1, PIN_RX2, PIN_TX2);
    Serial.println("[INIT] Serial CAM OK");
    
    // Servos
    servo1.attach(PIN_SERVO1);
    servo2.attach(PIN_SERVO2);
    servo3.attach(PIN_SERVO3);
    closeAllBins();
    Serial.println("[INIT] Servos OK");
    
    // LCD
    Wire.begin();
    lcd.init();
    lcd.backlight();
    lcdShowIdle();
    Serial.println("[INIT] LCD OK");
    
    Serial.println("[INIT] Ready!");
    Serial.println();
}

// ============================================================
// LOOP
// ============================================================

void loop() {
    runStateMachine();
    delay(10);
}
