# 🔧 ESP32 Smart Bin Hardware - Modular Architecture

## � TÀI LIỆU HƯỚNG DẪN

- **[QUICK_START.md](QUICK_START.md)** - Bắt đầu nhanh (dành cho người đã quen ESP32)
- **[HUONG_DAN_ARDUINO_IDE.md](HUONG_DAN_ARDUINO_IDE.md)** - Hướng dẫn chi tiết cài đặt Arduino IDE và nạp code
- **[HUONG_DAN_DAU_NOI.md](HUONG_DAN_DAU_NOI.md)** - Hướng dẫn đấu nối phần cứng từng bước
- **[README.md](README.md)** (file này) - Tổng quan dự án và cấu trúc code

---

## �📁 Cấu trúc thư mục

```
esp32_hardware/
├── README.md                      # Tài liệu này
├── shared/
│   └── protocol.h                 # Protocol chung giữa 2 boards
│
├── esp32_controller/              # ESP32 DevKit - Main Controller
│   ├── esp32_controller.ino       # Entry point
│   ├── config.h                   # ⚙️ CẤU HÌNH (CHỈNH SỬA Ở ĐÂY)
│   ├── pins.h                     # Pin definitions
│   ├── distance_sensor.h/.cpp     # Module: HC-SR04
│   ├── servo_controller.h/.cpp    # Module: 3 Servos
│   ├── led_status.h/.cpp          # Module: LED indicator
│   ├── lcd_display.h/.cpp         # Module: LCD 16x2 I2C
│   ├── serial_comm.h/.cpp         # Module: Serial communication
│   ├── state_machine.h            # State definitions
│   └── protocol.h                 # Protocol (copy from shared)
│
└── esp32_cam/                     # ESP32-CAM - Camera & Network
    ├── esp32_cam.ino              # Entry point
    ├── config.h                   # ⚙️ CẤU HÌNH (WiFi, Server URL!)
    ├── camera_pins.h              # Camera pin definitions
    ├── wifi_manager.h/.cpp        # Module: WiFi connection
    ├── camera_handler.h/.cpp      # Module: Camera operations
    ├── http_client.h/.cpp         # Module: HTTP requests
    ├── serial_comm.h/.cpp         # Module: Serial communication
    └── protocol.h                 # Protocol (copy from shared)
```

---

## 🔌 SƠ ĐỒ ĐẤU NỐI CHI TIẾT

### Tổng quan hệ thống

```
                                    ┌─────────────────────┐
                                    │    SERVER (EC2)     │
                                    │  api.smartbin.live  │
                                    └─────────┬───────────┘
                                              │ HTTPS (Internet)
                                              │
┌─────────────────────────────────────────────┼─────────────────────────────────────────────┐
│                                             │                                             │
│  ┌──────────────────────┐                   │                   ┌──────────────────────┐  │
│  │   ESP32-CAM Module   │                   │                   │   ESP32 Controller   │  │
│  │   (Camera + WiFi)    │◄──────────────────┘                   │    (Main Brain)      │  │
│  │                      │                                       │                      │  │
│  │  TX (G1) ────────────┼───────────────────────────────────────┼──► RX (G16)         │  │
│  │  RX (G3) ◄───────────┼───────────────────────────────────────┼─── TX (G17)         │  │
│  │  GND ────────────────┼───────────────────────────────────────┼──► GND              │  │
│  │                      │          Serial UART 115200           │                      │  │
│  └──────────────────────┘                                       │                      │  │
│                                                                 │  G18 ──► TRIG        │  │
│                                                                 │  G19 ◄── ECHO        │  │
│                                                                 │         (HC-SR04)    │  │
│                                                                 │                      │  │
│                                                                 │  G12 ──► Servo Đỏ   │  │
│                                                                 │  G26 ──► Servo Xanh │  │
│                                                                 │  G32 ──► Servo Xám  │  │
│                                                                 │                      │  │
│                                                                 │  G21 ──► LCD SDA    │  │
│                                                                 │  G22 ──► LCD SCL    │  │
│                                                                 │                      │  │
│                                                                 │  G2  ──► LED Status │  │
│                                                                 └──────────────────────┘  │
│                                                                                           │
│                                           NGUỒN 5V (2-3A)                                 │
└───────────────────────────────────────────────────────────────────────────────────────────┘
```

---

## 🔗 HƯỚNG DẪN NỐI ESP32-CAM VỚI ESP32 CONTROLLER

### Bước 1: Xác định chân trên ESP32-CAM

ESP32-CAM (AI-Thinker) có 2 hàng chân. Tìm các chân sau:

**⚠️ LƯU Ý: Trên board thực tế, TX/RX được ghi là UOT/UOR hoặc U0T/U0R**

```
      Top View (Camera lens ở trên)
    ┌─────────────────────────────┐
    │        [Camera Lens]        │
    ├─────────────────────────────┤
    │                             │
 5V ─┤ ○                     ○ ├── 3V3
GND ─┤ ○                     ○ ├── IO16
IO12─┤ ○                     ○ ├── IO0    ← GPIO0 (boot mode)
IO13─┤ ○                     ○ ├── GND
IO15─┤ ○                     ○ ├── VCC
IO14─┤ ○                     ○ ├── UOR    ← RX (GPIO3) - nối với ESP32 TX
 IO2─┤ ○                     ○ ├── UOT    ← TX (GPIO1) - nối với ESP32 RX
 IO4─┤ ○                     ○ ├── GND
    │                             │
    └─────────────────────────────┘

Ký hiệu trên board:        Tên GPIO:        Chức năng:
- UOT (hoặc U0T)    =      GPIO1      =     TX (truyền)
- UOR (hoặc U0R)    =      GPIO3      =     RX (nhận)
```

### Bước 2: Xác định chân trên ESP32 DevKit

```
    ESP32 DevKit (30 pins)
    ┌───────────────────────────┐
    │         [USB Port]        │
    ├───────────────────────────┤
 3.3V──┤ ○                   ○ ├──VIN (5V)
  EN ──┤ ○                   ○ ├──GND
 G36 ──┤ ○                   ○ ├──G23
 G39 ──┤ ○                   ○ ├──G22  ← LCD SCL
 G34 ──┤ ○                   ○ ├──G1   
 G35 ──┤ ○                   ○ ├──G3   
 G32 ──┤ ○  ← Servo Xám     ○ ├──G21  ← LCD SDA
 G33 ──┤ ○                   ○ ├──GND
 G25 ──┤ ○                   ○ ├──G19  ← HC-SR04 ECHO
 G26 ──┤ ○  ← Servo Xanh    ○ ├──G18  ← HC-SR04 TRIG
 G27 ──┤ ○                   ○ ├──G5
 G14 ──┤ ○                   ○ ├──G17  ← Serial TX → CAM RX
 G12 ──┤ ○  ← Servo Đỏ      ○ ├──G16  ← Serial RX ← CAM TX
 GND ──┤ ○                   ○ ├──G4
 G13 ──┤ ○                   ○ ├──G2   ← LED Status
    │         [   ]             │
    └───────────────────────────┘
```

### Bước 3: Đấu nối Serial giữa 2 boards

**⚠️ QUAN TRỌNG: TX nối với RX (chéo), KHÔNG PHẢI TX nối TX!**

```
ESP32 Controller              ESP32-CAM (ký hiệu trên board)
────────────────              ───────────────────────────────
GPIO17 (TX) ─────────────────► UOR (RX)
GPIO16 (RX) ◄───────────────── UOT (TX)
GND         ─────────────────► GND

Dây cần dùng: 3 sợi jumper (đực-cái hoặc cái-cái)
```

**Hình ảnh minh họa:**

```
     ESP32 DevKit                           ESP32-CAM
    ┌────────────┐                        ┌────────────┐
    │            │   Dây XANH (TX→RX)     │            │
    │    G17 ────┼───────────────────────►│──── UOR    │
    │            │                        │            │
    │    G16 ◄───┼────────────────────────┼──── UOT    │
    │            │   Dây VÀNG (RX←TX)     │            │
    │            │                        │            │
    │    GND ────┼────────────────────────┼──── GND    │
    │            │   Dây ĐEN (GND chung)  │            │
    └────────────┘                        └────────────┘
```

### Bước 4: Đấu nối nguồn

**⚠️ ESP32-CAM cần nguồn 5V ổn định (ít nhất 500mA)**

```
Nguồn 5V (2-3A)
     │
     ├────► VIN của ESP32 DevKit
     │
     └────► 5V của ESP32-CAM
     
GND (nguồn) ──► GND chung của cả 2 boards
```

**Nếu dùng USB laptop:**
- ESP32 DevKit: Cắm USB trực tiếp
- ESP32-CAM: Cấp nguồn 5V riêng (không đủ dòng nếu dùng chung USB)

---

## 📋 BẢNG TÓM TẮT ĐẤU NỐI

### ESP32 Controller Pins:

| Chức năng | GPIO | Kết nối đến |
|-----------|------|-------------|
| **Serial Communication** | | |
| RX (nhận từ CAM) | G16 | ESP32-CAM TX (G1) |
| TX (gửi đến CAM) | G17 | ESP32-CAM RX (G3) |
| **HC-SR04** | | |
| TRIG | G18 | HC-SR04 TRIG |
| ECHO | G19 | HC-SR04 ECHO |
| **Servos** | | |
| Thùng Đỏ (Bin 1) | G12 | Servo signal |
| Thùng Xanh (Bin 2) | G26 | Servo signal |
| Thùng Xám (Bin 3) | G32 | Servo signal |
| **LCD I2C** | | |
| SDA | G21 | LCD SDA |
| SCL | G22 | LCD SCL |
| **LED** | | |
| Status LED | G2 | LED (+) → 220Ω → GND |

### ESP32-CAM Pins:

| Ký hiệu trên board | GPIO | Kết nối đến |
|-------------------|------|-------------|
| **UOT** (hoặc U0T) | GPIO1 - TX | ESP32 Controller RX (G16) |
| **UOR** (hoặc U0R) | GPIO3 - RX | ESP32 Controller TX (G17) |
| 5V | 5V pin | Nguồn 5V |
| GND | GND | GND chung |
| IO4 | GPIO4 | Flash LED (internal) |

---

## ⚡ CÀI ĐẶT THƯ VIỆN

### Arduino IDE → Tools → Manage Libraries:

| Thư viện | Version | Cho board |
|----------|---------|-----------|
| **ESP32Servo** | ≥1.0.0 | ESP32 Controller |
| **LiquidCrystal I2C** (by Frank de Brabander) | ≥1.1.2 | ESP32 Controller |
| **ArduinoJson** | ≥6.21.0 | ESP32-CAM |

---

## 🎯 CÁCH SỬ DỤNG

### 1. Cấu hình (CHỈ CẦN SỬA 2 FILE config.h)

**ESP32-CAM** → Mở `esp32_cam/config.h`:
```cpp
#define WIFI_SSID       "TenWiFiCuaBan"
#define WIFI_PASSWORD   "MatKhauWiFi"
#define SERVER_URL      "https://api.smartbin.live/classify"
```

**ESP32 Controller** → Mở `esp32_controller/config.h`:
```cpp
#define OBJECT_DISTANCE_CM   20    // Khoảng cách phát hiện (cm)
#define SERVO_OPEN_ANGLE     90    // Góc mở nắp
#define LID_OPEN_DURATION_MS 5000  // Thời gian giữ nắp mở (ms)
```

### 2. Upload Code

**Bước 1**: Upload ESP32 Controller
```
1. Mở Arduino IDE
2. File → Open → esp32_controller/esp32_controller.ino
3. Tools → Board: "ESP32 Dev Module"
4. Tools → Port: COMx
5. Click Upload ✅
```

**Bước 2**: Upload ESP32-CAM
```
1. NGẮT dây Serial nối với ESP32 Controller (để không conflict)
2. Cắm ESP32-CAM vào MB Programmer board (nếu có)
   HOẶC nối FTDI:
   - FTDI TX → CAM RX (G3)
   - FTDI RX → CAM TX (G1)
   - FTDI GND → CAM GND
3. NỐI GPIO0 → GND (vào boot mode)
4. Cấp nguồn / Reset
5. Mở Arduino IDE
6. File → Open → esp32_cam/esp32_cam.ino
7. Tools → Board: "AI Thinker ESP32-CAM"
8. Tools → Port: COMx (của FTDI/Programmer)
9. Click Upload
10. Khi thấy "Connecting...", nhấn nút Reset trên CAM
11. Đợi upload xong
12. NGẮT GPIO0 khỏi GND
13. RÚT ESP32-CAM ra khỏi Programmer
14. NỐI LẠI dây Serial với ESP32 Controller
15. Cấp nguồn → Hệ thống hoạt động! ✅
```

### 3. Mapping loại rác → thùng màu

Mặc định trong code:

| Loại rác (từ server) | Bin Number | Thùng màu |
|----------------------|------------|-----------|
| paper (giấy) | 1 | Đỏ |
| plastic (nhựa) | 2 | Xanh |
| glass (thủy tinh) | 3 | Xám |

**Để thay đổi mapping**, sửa trong `servo_controller.h`:
```cpp
#define BIN_PAPER       BIN_RED     // paper → thùng đỏ
#define BIN_PLASTIC     BIN_GREEN   // plastic → thùng xanh
#define BIN_GLASS       BIN_GRAY    // glass → thùng xám
```

---

## 🐛 DEBUG

Mở Serial Monitor (115200 baud) để xem logs:

```
========================================
    Smart Bin Controller v1.0.0
========================================

[INIT] Starting initialization...
[INIT] Distance sensor... OK
[INIT] Servo controller... OK
[INIT] LED status... OK
[INIT] Serial communication... OK
[INIT] LCD display... OK

[INIT] All systems ready!
[INIT] Waiting for objects...

[DISTANCE] 18cm detected
[STATE] IDLE → OBJECT_DETECTED
[STATE] Stability count: 1/3
[STATE] Stability count: 2/3
[STATE] Stability count: 3/3
[STATE] OBJECT_DETECTED → WAITING_STABLE
[STATE] WAITING_STABLE → REQUESTING_CAPTURE
[SERIAL] TX: CAPTURE
[STATE] REQUESTING_CAPTURE → WAITING_RESULT
[SERIAL] RX: BIN:2
[STATE] WAITING_RESULT → OPENING_BIN
[SERVO] Opening: Thùng Xanh
[SERVO] Waste type: Plastic
[STATE] OPENING_BIN → HOLDING_OPEN
... (5 giây giữ nắp mở)
[STATE] HOLDING_OPEN → CLOSING_BIN
[SERVO] Closing: Thùng Xanh
[STATE] CLOSING_BIN → COOLDOWN
[STATE] COOLDOWN → IDLE
```

---

## 🔧 XỬ LÝ SỰ CỐ

### LCD không hiển thị
1. Kiểm tra địa chỉ I2C: Thử đổi `0x27` thành `0x3F` trong `pins.h`
2. Kiểm tra dây SDA/SCL có đúng không
3. Chạy I2C Scanner sketch để tìm địa chỉ đúng

### Servo không quay
1. Kiểm tra nguồn 5V (≥2A cho 3 servo)
2. GND servo phải nối với GND chung
3. Kiểm tra chân GPIO đúng theo `pins.h`

### ESP32-CAM không giao tiếp
1. Kiểm tra TX-RX đấu chéo
2. Kiểm tra GND chung
3. Kiểm tra baud rate = 115200 ở cả 2 boards
4. **Tắt debug log trên ESP32-CAM** (xem [DEBUG_SERIAL_COMMUNICATION.md](DEBUG_SERIAL_COMMUNICATION.md))
5. Thử gửi "PING" từ Serial Monitor → phải nhận "PONG"

### HC-SR04 không phát hiện
1. Kiểm tra TRIG (G18) và ECHO (G19)
2. VCC sensor = 5V (hoặc 3.3V tùy module)
3. Test bằng code đơn giản đọc khoảng cách

---

## 📦 Thư viện đầy đủ cần cài

```
ESP32Servo              - Điều khiển servo với ESP32
LiquidCrystal I2C       - LCD 16x2 I2C
ArduinoJson             - Parse JSON response từ server
```
