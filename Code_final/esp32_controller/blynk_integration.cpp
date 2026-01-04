/**
 * @file blynk_integration.cpp
 * @brief Blynk IoT - Điều khiển servo + Thông báo rác đầy
 */

#define BLYNK_TEMPLATE_ID   "TMPL63afuGzmK"
#define BLYNK_TEMPLATE_NAME "Thungracthongminh"
#define BLYNK_AUTH_TOKEN    "0D4LK3MQhDcpVVSR4OMC7PdWsCv12Rh3"

#include "blynk_integration.h"
#include "servo_controller.h"
#include "state_machine.h"
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Biến extern từ esp32_controller.ino
extern SystemState currentState;

// ============================================================
// CẢM BIẾN MỨC RÁC
// ============================================================
static const int trigPins[] = {PIN_TRIG_RED, PIN_TRIG_GREEN, PIN_TRIG_GRAY};
static const int echoPins[] = {PIN_ECHO_RED, PIN_ECHO_GREEN, PIN_ECHO_GRAY};
static const char* tenThung[] = {"Đỏ", "Xanh", "Xám"};
static bool daThongBao[] = {false, false, false};

static BlynkTimer timer;

// Đo khoảng cách HC-SR04
static int layKhoangCach(int i)
{
    digitalWrite(trigPins[i], LOW);
    delayMicroseconds(2);
    digitalWrite(trigPins[i], HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPins[i], LOW);
    long duration = pulseIn(echoPins[i], HIGH, 30000);
    return duration * 0.034 / 2;
}

// Kiểm tra mức rác (gọi từ timer)
static void checkTrashLevel()
{
    Serial.print("[TRASH] ");
    for (int i = 0; i < 3; i++)
    {
        int distance = layKhoangCach(i);
        Serial.printf("%s:%dcm ", tenThung[i], distance);
        
        // distance = 0 có nghĩa là cảm biến không nhận được echo (lỗi hoặc quá xa)
        if (distance > 0 && distance <= TRASH_FULL_CM)
        {
            if (!daThongBao[i])
            {
                Serial.printf("\n[ALERT] >>> Thung %s DAY! Dang gui thong bao...\n", tenThung[i]);
                String msg = "Thùng rác " + String(tenThung[i]) + " đã đầy!";
                Blynk.logEvent("rac_day", msg);
                daThongBao[i] = true;
            }
        }
        else if (distance > TRASH_RESET_CM)
        {
            if (daThongBao[i])
            {
                Serial.printf("\n[INFO] Thung %s da RESET (>%dcm)\n", tenThung[i], TRASH_RESET_CM);
            }
            daThongBao[i] = false;
        }
    }
    Serial.println();
}

// ============================================================
// KIỂM TRA HỆ THỐNG CÓ ĐANG BẬN KHÔNG
// ============================================================
static bool isSystemBusy()
{
    return (currentState != STATE_IDLE && currentState != STATE_COOLDOWN);
}

// ============================================================
// BLYNK CALLBACKS - Điều khiển Servo từ App
// ============================================================

BLYNK_WRITE(V1) {
    if (isSystemBusy()) {
        Serial.println("[BLYNK] Blocked! System is processing...");
        return;
    }
    if (param.asInt() == 1) servoController_openBin(1);
    else servoController_closeBin(1);
}

BLYNK_WRITE(V2) {
    if (isSystemBusy()) {
        Serial.println("[BLYNK] Blocked! System is processing...");
        return;
    }
    if (param.asInt() == 1) servoController_openBin(2);
    else servoController_closeBin(2);
}

BLYNK_WRITE(V3) {
    if (isSystemBusy()) {
        Serial.println("[BLYNK] Blocked! System is processing...");
        return;
    }
    if (param.asInt() == 1) servoController_openBin(3);
    else servoController_closeBin(3);
}

// ============================================================
// PUBLIC FUNCTIONS
// ============================================================

bool blynk_init()
{
    // Setup chân cảm biến mức rác
    for (int i = 0; i < 3; i++)
    {
        pinMode(trigPins[i], OUTPUT);
        pinMode(echoPins[i], INPUT);
    }
    Serial.println("[BLYNK] Trash level sensors initialized");
    
    Serial.println("[BLYNK] Connecting to WiFi...");
    WiFi.begin(BLYNK_WIFI_SSID, BLYNK_WIFI_PASS);
    
    int timeout = 20;
    while (WiFi.status() != WL_CONNECTED && timeout > 0)
    {
        delay(500);
        Serial.print(".");
        timeout--;
    }
    
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println(" FAILED");
        return false;
    }
    
    Serial.println(" OK");
    Serial.print("[BLYNK] IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("[BLYNK] WiFi Channel: ");
    Serial.println(WiFi.channel());
    
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
    
    // Timer kiểm tra mức rác mỗi 2 giây
    timer.setInterval(2000L, checkTrashLevel);
    
    return true;
}

void blynk_update()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        Blynk.run();
        timer.run();  // Chạy timer kiểm tra mức rác
    }
}
