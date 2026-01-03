# ⚡ QUICK START - BẮT ĐẦU NHANH

Hướng dẫn rút gọn cho người đã quen với ESP32 và Arduino IDE.

## 📋 CHUẨN BỊ

### Phần cứng:
- 1x ESP32 DevKit (30 pin)
- 1x ESP32-CAM + MB Programmer
- 1x HC-SR04
- 3x Servo SG90/MG90S
- 1x LCD 16x2 I2C
- Nguồn 5V/3A

### Phần mềm:
- Arduino IDE 2.x
- ESP32 Board Support v2.0.14+
- Libraries: `ESP32Servo`, `LiquidCrystal I2C`, `ArduinoJson`

## 🚀 5 BƯỚC NHANH

### 1️⃣ Cài đặt Arduino IDE

```bash
# Board Manager URL:
https://espressif.github.io/arduino-esp32/package_esp32_index.json

# Cài ESP32 board package (Tools → Board Manager)
esp32 by Espressif Systems v2.0.14+

# Cài libraries (Tools → Manage Libraries)
- ESP32Servo ≥1.0.0
- LiquidCrystal I2C by Frank de Brabander ≥1.1.2
- ArduinoJson 6.x (≥6.21.0)
```

### 2️⃣ Cấu hình WiFi (chỉ ESP32-CAM)

Mở `esp32_cam/config.h`:

```cpp
#define WIFI_SSID       "TenWiFi"
#define WIFI_PASSWORD   "MatKhau"
#define SERVER_URL      "https://api.smartbin.live/classify"
```

### 3️⃣ Upload ESP32 Controller

```
File → Open → esp32_controller/esp32_controller.ino

Tools:
├── Board: ESP32 Dev Module
├── Upload Speed: 921600
├── Flash Size: 4MB
├── Partition Scheme: Default 4MB
└── Port: COMx hoặc /dev/ttyUSBx

Upload → Đợi hoàn tất
```

### 4️⃣ Upload ESP32-CAM

```
File → Open → esp32_cam/esp32_cam.ino

Tools:
├── Board: AI Thinker ESP32-CAM
├── Upload Speed: 921600
├── Partition Scheme: Huge APP (3MB No OTA)
└── Port: COMx (của MB Programmer)

Lắp CAM vào MB Programmer → Upload
Khi thấy "Connecting...", nhấn BOOT + RESET trên MB Programmer
```

### 5️⃣ Đấu nối và chạy

```
ESP32 Controller          ESP32-CAM
────────────────          ─────────
GPIO17 (TX) ─────────────► UOR (RX)
GPIO16 (RX) ◄──────────── UOT (TX)
GND ──────────────────────► GND

Cấp nguồn 5V → Hệ thống chạy!
```

## 🔍 KIỂM TRA NHANH

### Serial Monitor (115200 baud):

**ESP32 Controller:**
```
[INIT] All systems ready!
[INIT] Waiting for objects...
```

**ESP32-CAM:**
```
[WIFI] Connected! IP: 192.168.1.XXX
[CAMERA] Camera initialized successfully
```

## 📍 PINOUT NHANH

### ESP32 Controller:

| Module | Pin | Connect |
|--------|-----|---------|
| Serial RX | G16 | CAM UOT (TX) |
| Serial TX | G17 | CAM UOR (RX) |
| HC-SR04 TRIG | G18 | Sensor TRIG |
| HC-SR04 ECHO | G19 | Sensor ECHO |
| Servo Đỏ | G12 | Servo signal |
| Servo Xanh | G26 | Servo signal |
| Servo Xám | G32 | Servo signal |
| LCD SDA | G21 | LCD SDA |
| LCD SCL | G22 | LCD SCL |
| LED | G2 | LED (+) |

### ESP32-CAM:

| Board Label | GPIO | Connect |
|-------------|------|---------|
| UOT (U0T) | GPIO1 | Controller G16 |
| UOR (U0R) | GPIO3 | Controller G17 |
| 5V | - | Power 5V |
| GND | - | GND chung |

## ⚠️ LƯU Ý QUAN TRỌNG

1. **WiFi 2.4GHz only** - ESP32 không hỗ trợ 5GHz
2. **Nguồn đủ mạnh** - Ít nhất 5V/2A cho toàn bộ hệ thống
3. **GND chung** - Tất cả thiết bị phải nối GND chung
4. **TX ↔ RX** - Nhớ đấu chéo (TX → RX, RX ← TX)
5. **UOT/UOR** - Đây là ký hiệu thực tế trên ESP32-CAM board

## 🐛 LỖI THƯỜNG GẶP

| Lỗi | Giải pháp |
|-----|-----------|
| Failed to connect | Nhấn BOOT + RESET, thử Upload Speed 115200 |
| Sketch too big | Partition: Huge APP (3MB) cho CAM |
| WiFi failed | Kiểm tra SSID/Pass, đảm bảo 2.4GHz |
| Camera init failed | Kiểm tra flex cable cắm chặt |
| No serial output | Baud rate = 115200 |

## 📚 ĐỌC THÊM

- **Chi tiết Arduino IDE**: [HUONG_DAN_ARDUINO_IDE.md](HUONG_DAN_ARDUINO_IDE.md)
- **Chi tiết đấu nối**: [HUONG_DAN_DAU_NOI.md](HUONG_DAN_DAU_NOI.md)
- **Cấu trúc code**: [README.md](README.md)

## 🎯 TEST NHANH

```cpp
// Test Serial Communication:
// 1. Upload code cho cả 2 board
// 2. Nối Serial: G17→UOR, G16←UOT, GND chung
// 3. Mở Serial Monitor của Controller
// 4. Gõ: PING
// 5. Nhận được: PONG → OK ✅

// Test Distance Sensor:
// Đặt tay 10-20cm trước sensor
// Serial Monitor: [DISTANCE] 15cm detected

// Test Servo:
// Đặt rác vào → Servo mở → 5s → Servo đóng

// Test LCD:
// Hiển thị: "Cho rac vao..." khi idle
```

## ✅ CHECKLIST

- [ ] Arduino IDE cài xong + ESP32 board package
- [ ] 3 thư viện đã cài: ESP32Servo, LiquidCrystal I2C, ArduinoJson
- [ ] WiFi SSID/Password đã điền vào esp32_cam/config.h
- [ ] Upload ESP32 Controller thành công
- [ ] Upload ESP32-CAM thành công
- [ ] Đấu nối Serial: G17↔UOR, G16↔UOT, GND chung
- [ ] Các module: HC-SR04, Servos, LCD đã nối đúng pins
- [ ] Nguồn 5V/2A đã kết nối
- [ ] Serial Monitor thấy log khởi động
- [ ] Test PING/PONG hoạt động
- [ ] Hệ thống phát hiện và phân loại rác OK

---

**🎉 Done! Chúc bạn thành công!**
