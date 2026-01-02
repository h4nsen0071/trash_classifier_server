# Hướng Dẫn Triển Khai ESP32-CAM Smart Trash Bin

## 📋 Mục Lục
1. [Chuẩn Bị Phần Cứng](#1-chuẩn-bị-phần-cứng)
2. [Cài Đặt Phần Mềm](#2-cài-đặt-phần-mềm)
3. [Cấu Hình Code](#3-cấu-hình-code)
4. [Kết Nối Phần Cứng](#4-kết-nối-phần-cứng)
5. [Upload Code](#5-upload-code)
6. [Testing & Debug](#6-testing--debug)
7. [Troubleshooting](#7-troubleshooting)

---

## 1. Chuẩn Bị Phần Cứng

### Linh Kiện Cần Thiết

| Linh kiện | Số lượng | Ghi chú |
|-----------|----------|---------|
| ESP32-CAM với MB (đế nạp) | 1 | Module chính + USB adapter |
| PIR Sensor HC-SR501 | 1 | Phát hiện chuyển động |
| Ultrasonic HC-SR04 | 1 | Đo khoảng cách |
| Servo Motor SG90 | 3 | Điều khiển 3 nắp thùng |
| Nguồn 5V 2A | 1 | Cấp nguồn cho hệ thống |
| Breadboard + Dây jumper | 1 set | Kết nối |
| Cáp USB Type-C hoặc Micro-USB | 1 | Tùy loại MB |

### Lưu Ý Nguồn
- ESP32-CAM tiêu thụ 200-500mA khi chụp ảnh
- 3 Servo: ~500mA mỗi cái
- **Tổng**: Cần nguồn 5V ≥2A ổn định

---

## 2. Cài Đặt Phần Mềm

### Bước 1: Cài Arduino IDE
1. Download Arduino IDE 2.x: https://www.arduino.cc/en/software
2. Cài đặt và mở Arduino IDE

### Bước 2: Cài ESP32 Board Manager
1. Mở **File → Preferences**
2. Thêm URL vào **Additional Boards Manager URLs**:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Mở **Tools → Board → Boards Manager**
4. Tìm "ESP32" và cài đặt **esp32 by Espressif Systems**
5. Chọn board: **Tools → Board → ESP32 Arduino → AI Thinker ESP32-CAM**

### Bước 3: Cài Libraries
Vào **Tools → Manage Libraries**, tìm và cài:

| Library | Version | Cách cài |
|---------|---------|----------|
| ArduinoJson | 6.x | Library Manager → Search "ArduinoJson" |
| ESP32Servo | Latest | Library Manager → Search "ESP32Servo" |

**Lưu ý**: WiFi.h, HTTPClient.h, esp_camera.h đã có sẵn trong ESP32 board package.

---

## 3. Cấu Hình Code

Mở file `esp32_smart_bin.ino` và chỉnh sửa:

### 3.1. WiFi & Server (Dòng 11-17)

```cpp
// WiFi credentials
const char* WIFI_SSID = "YOUR_WIFI_SSID";        // → Thay bằng tên WiFi nhà bạn
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // → Thay bằng mật khẩu WiFi

// Server URL - Chọn 1 trong 2:

// Option 1: Development - Server chạy local
const char* SERVER_URL = "http://192.168.1.100:5000/classify";

// Option 2: Production - Server trên EC2 với Cloudflare Tunnel
const char* SERVER_URL = "https://trash-classifier.yourdomain.com/classify";
```

**Cách chọn**:
- **Local development**: Dùng HTTP với IP local (ví dụ: 192.168.1.100)
  - Tìm IP server: Windows `ipconfig`, Linux/Mac `ifconfig`
  - Server và ESP32 phải cùng mạng WiFi
  
- **Production deployment**: Dùng HTTPS với domain Cloudflare Tunnel
  - Không cần biết IP server
  - ESP32 gọi qua domain → Cloudflare Tunnel → Server
  - Bảo mật hơn với HTTPS

### 3.2. GPIO Pins (Dòng 16-23)

**Lưu ý**: ESP32-CAM chỉ có một số GPIO khả dụng!

```cpp
// Pins KHÔNG được dùng trên ESP32-CAM (đã dùng cho camera):
// GPIO 0, 4, 5, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 34, 35, 36, 39

// Pins KHẢ DỤNG:
// GPIO 1 (TX), 3 (RX), 2, 12, 13, 14, 15, 16
```

**Cấu hình mẫu (An toàn)**:
```cpp
#define PIR_PIN 13          // GPIO 13 - PIR sensor
#define TRIG_PIN 14         // GPIO 14 - Ultrasonic trigger
#define ECHO_PIN 15         // GPIO 15 - Ultrasonic echo
#define SERVO_PAPER_PIN 12  // GPIO 12 - Servo ngăn giấy
#define SERVO_GLASS_PIN 2   // GPIO 2  - Servo ngăn thủy tinh
#define SERVO_PLASTIC_PIN 16 // GPIO 16 - Servo ngăn nhựa
```

**⚠️ Cảnh báo GPIO**:
- GPIO 0: Dùng để boot mode (KHÔNG nối servo/sensor)
- GPIO 1/3: TX/RX UART (nếu dùng thì không debug Serial)
- GPIO 12: Boot fail nếu HIGH khi khởi động

### 3.3. System Parameters (Dòng 53-58)

```cpp
#define DISTANCE_THRESHOLD 30    // cm - Khoảng cách trigger
#define STABLE_TIME 500          // ms - Thời gian ổn định
#define CAPTURE_INTERVAL 200     // ms - Khoảng cách giữa frames
#define NUM_FRAMES 3             // Số frame chụp
#define BIN_OPEN_DURATION 3000   // ms - Thời gian mở nắp
#define COOLDOWN_TIME 5000       // ms - Cooldown giữa các lần
```

**Điều chỉnh theo nhu cầu**:
- `DISTANCE_THRESHOLD`: 20-50cm tùy kích thước thùng
- `STABLE_TIME`: Tăng lên 800-1000ms nếu nhiễu
- `BIN_OPEN_DURATION`: Tăng nếu người dùng bỏ rác chậm

---

## 4. Kết Nối Phần Cứng

### 4.1. Sơ Đồ Kết Nối

**Lưu ý**: ESP32-CAM đã có đế nạp (MB) tích hợp USB, không cần FTDI riêng!

```
ESP32-CAM + MB     Máy tính
--------------     ---------
USB Port   ──────  Cáp USB (upload & power khi test)

ESP32-CAM          PIR HC-SR501
---------          -------------
5V       ────────  VCC
GND      ────────  GND
GPIO13   ────────  OUT

ESP32-CAM          Ultrasonic HC-SR04
---------          --------------------
5V       ────────  VCC
GND      ────────  GND
GPIO14   ────────  TRIG
GPIO15   ────────  ECHO

ESP32-CAM          Servo Motor (x3)
---------          ----------------
5V       ────────  VCC (Red)
GND      ────────  GND (Brown/Black)
GPIO12   ────────  Signal (Orange/Yellow) - Paper
GPIO2    ────────  Signal - Glass
GPIO16   ────────  Signal - Plastic
```

### 4.2. Lưu Ý Kết Nối

**1. Nguồn Servo riêng biệt** (Khuyến nghị):
```
Nguồn 5V → Regulator 5V → Servo VCC (x3)
                        → ESP32-CAM 5V
          → GND chung
```

**2. Chia áp cho RX pin** (Nếu dùng FTDI 5V logic):
```
FTDI Khi Upload Code**:
- Cắm USB trực tiếp vào đế MB (có sẵn)
- Không cần nối GPIO0 thủ công

**3. Khi Chạy Thực Tế**:
- Tháo ESP32-CAM ra khỏi đế MB
- Cấp nguồn 5V vào chân 5V và GND
- Hoặc giữ nguyên trên MB và cấp nguồn qua USB

**4
---
USB

1. Cắm ESP32-CAM vào đế nạp MB (nếu chưa)
2. Cắm cáp USB từ MB vào máy tính
3. Đợi Windows nhận diện thiết bị (driver CH340/CP2102 tự cài)
1. Nối ESP32-CAM với FTDI như sơ đồ trên
2. **Quan trọng**: Nối GPIO0 → GND (để vào Flash Mode)
3. Cắm FTDI vào USB máy tính

### Bước 2: Cấu Hình Arduino IDE

```
Tools → Board: "AI Thinker ESP32-CAM"
Tools → Upload Speed: "115200"
Tools → CPU Frequency: "240MHz"
Tools → Flash Frequency: "80MHz"
Tools → Flash Mode: "QIO"
Tools → Partition Scheme: "Huge APP (3MB No OTA)"
Tools → Port: COM3 (Windows) hoặc /dev/ttyUSB0 (Linux)
```
**Với đế nạp MB** (đơn giản hơn):
1. Click **Upload** (hoặc Ctrl+U)
2. Chờ "Connecting..." → Code tự upload (MB tự vào flash mode)
3. Chờ upload hoàn tất (100%)
4. Nhấn nút **RST** trên MB để khởi động

**Nếu lỗi "Connecting..."**:
- Giữ nút **BOOT** trên MB
- Click **Upload** trong Arduino IDE
- Nhả nút BOOT khi thấy "Writing at 0x..."nút **RESET** trên ESP32-CAM
3. Chờ upload hoàn tất (100%)
4. **Tháo GPIO0 khỏi GND**
5. Nhấn **RESET** để khởi động
nút **RST** trên MB
4. Xem log:
   ```
   WiFi: 192.168.1.123
   System ready
   ```

**Lưu ý**: Khi chạy thực tế (không debug), có thể tháo ESP32-CAM ra khỏi MB và cấp nguồn riêng.n RESET trên ESP32-CAM
4. Xem log:
   ```
   WiFi: 192.168.1.123
   System ready
   ```

---

## 6. Testing & Debug

### Test 1: WiFi Connection
```
Expected: "WiFi: 192.168.1.xxx"
Error: "WiFi connection failed" → Kiểm tra SSID/Password
```

### Test 2: Camera Init
```
Expected: Camera khởi động thành công
Error: "Camera init failed" → Kiểm tra nguồn 5V ổn định
```

### Test 3: PIR Sensor
```
Test: Vẫy tay trước PIR
Expected: Serial log "Motion detected"
```

### Test 4: Distance Sensor
```
Test: Đưa tay gần cảm biến (<30cm)
Expected: "Distance stable, capturing..."
```

### Test 5: Capture & Send
```
Test: Trigger bằng PIR + Distance
Expected: "Capturing 3 frames..." → Gửi server → Nhận response
```

### Test 6: Servo Control
```
Test: Sau khi nhận response từ server
Expected: Servo mở ngăn tương ứng 3 giây → đóng
```

---

## 7. Troubleshooting

### Lỗi 1: "Camera init failed"
**Nguyên nhân**:
- Nguồn 5V yếu (<500mA)
- Camera module lỏng

**Giải pháp**:
- Dùng nguồn 5V 2A
- Kiểm tra camera ribbon cable
- Thêm tụ 100µF

### Lỗi 2: "WiFi connection failed"
**Nguyên nhân**:
- SSID/Password sai
- WiFi 5GHz (ESP32 chỉ hỗ trợ 2.4GHz)

**Giải pháp**:
- Kiểm tra SSID/Password (phân biệt hoa/thường)
- Dùng WiFi 2.4GHz

### Lỗi 3: "HTTP Error"
**Nguyên nhân**:
- Server chưa chạy
- Server URL sai
- Firewall chặn port 5000

**Giải pháp**:
```bash
# Kiểm tra server chạy
curl http://192.168.1.50:5000/health

# Tắt firewall tạm (Windows)
netsh advfirewall set allprofiles state off
```

### Lỗi 4: Servo không quay
**Nguyên nhân**:
- GPIO pins sai
- Nguồn servo không đủ

**Giải pháp**:
- Kiểm tra GPIO pins (không dùng GPIO camera)
- Cấp nguồn riêng cho servo

### Lỗi 5: ESP32 reset liên tục
**Nguyên nhân**:
- Watchdog timeout
- Nguồn không ổn định

**Giải pháp**:
- Thêm `delay(10)` trong loop
- Dùng nguồn 5V ổn định

### Lỗi 6: PIR trigger liên tục
**Nguyên nhân**:
- PIR sensitivity quá cao
- PIR chưa ổn định sau power on

**Giải pháp**:
- Chờ 30-60s sau khi bật nguồn
- Xoay biến trở sensitivity trên PIR

### Lỗi 7: Distance sensor trả về -1
**Nguyên nhân**:
- Echo timeout
- Vật cản quá xa/gần
- Góc phản xạ sai

**Giải pháp**:
- Test trong phạm vi 5-200cm
- Đặt sensor vuông góc với vật

---

## 8. Deployment Checklist

### Trước Khi Triển Khai

- [ ] Đã test toàn bộ chức năng trên breadboard
- [ ] Server Flask đang chạy và accessible
- [ ] WiFi coverage tốt tại vị trí thùng rác
- [ ] Nguồn 5V 2A ổn định
- [ ] Tất cả servo hoạt động mượt mà
- [ ] PIR và Distance sensor đã calibrate

### Sau Khi Lắp Đặt

- [ ] Kiểm tra Serial log qua USB (debug mode)
- [ ] Test 5-10 lần liên tục
- [ ] Đo thời gian phản hồi (PIR trigger → Bin open)
- [ ] Kiểm tra false positive rate
- [ ] Monitor battery/power consumption

### Optimization

- [ ] Điều chỉnh `DISTANCE_THRESHOLD` theo thực tế
- [ ] Tăng `STABLE_TIME` nếu nhiễu nhiều
- [ ] Điều chỉnh `COOLDOWN_TIME` theo mật độ sử dụng
- [ ] Tối ưu `CAPTURE_INTERVAL` và `NUM_FRAMES`

---

## 9. Bảo Trì

### Hàng Tuần
- Kiểm tra log errors
- Test PIR sensitivity
- Vệ sinh lens camera

### Hàng Tháng
- Backup và phân tích logs
- Kiểm tra độ chính xác phân loại
- Update model nếu cần

### Khi Có Lỗi
1. Kiểm tra Serial log
2. Restart ESP32 (nhấn RESET)
3. Kiểm tra kết nối WiFi
4. Ping server
5. Re-upload code nếu cần

---

## 10. Contact & Support

**Lỗi phần cứng**: Kiểm tra kết nối GPIO, nguồn  
**Lỗi mạng**: Kiểm tra WiFi, Server URL  
**Lỗi phân loại**: Kiểm tra server Flask, model

Good luck! 🚀
