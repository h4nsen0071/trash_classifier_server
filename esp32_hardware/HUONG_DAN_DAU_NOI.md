# 📋 HƯỚNG DẪN ĐẤU NỐI CHI TIẾT

## 📦 VẬT TƯ CẦN CHUẨN BỊ

### Board mạch:
- 1x ESP32 DevKit (30 pin) - Controller chính
- 1x ESP32-CAM (AI-Thinker) - Camera module
- 1x MB Programmer hoặc FTDI adapter (để nạp code cho ESP32-CAM)

### Cảm biến & module:
- 1x HC-SR04 (cảm biến siêu âm)
- 3x Servo SG90/MG90S (điều khiển nắp thùng)
- 1x LCD 16x2 I2C (hiển thị trạng thái)
- 1x LED (tùy chọn - trạng thái)

### Dây nối:
- ~20 sợi jumper đực-cái
- ~10 sợi jumper cái-cái
- 3 sợi dây servo extension (nếu cần kéo dài)

### Nguồn:
- 1x Nguồn 5V/3A (quan trọng!)
- Hoặc: 1x PowerBank 5V/2A + 1x adapter 5V/2A

---

## 🔌 BẢNG ĐẤU NỐI TỔNG HỢP

### A. ESP32 CONTROLLER (DevKit)

| GPIO | Chức năng | Nối đến | Màu dây gợi ý |
|------|-----------|---------|---------------|
| **Serial** | | | |
| G16 | RX | ESP32-CAM UOT (TX) | Vàng |
| G17 | TX | ESP32-CAM UOR (RX) | Xanh lá |
| **HC-SR04** | | | |
| G18 | TRIG | HC-SR04 TRIG | Cam |
| G19 | ECHO | HC-SR04 ECHO | Tím |
| **Servo** | | | |
| G12 | PWM | Servo Đỏ (signal) | Đỏ |
| G26 | PWM | Servo Xanh (signal) | Xanh dương |
| G32 | PWM | Servo Xám (signal) | Xám |
| **LCD I2C** | | | |
| G21 | SDA | LCD SDA | Trắng |
| G22 | SCL | LCD SCL | Xanh lam |
| **LED** | | | |
| G2 | LED | LED (+) → 220Ω → GND | Đỏ |
| **Nguồn** | | | |
| VIN | 5V | Nguồn 5V (+) | Đỏ |
| GND | Ground | Nguồn GND (-) | Đen |

### B. ESP32-CAM

**⚠️ Trên board thực tế, TX/RX được ghi là UOT/UOR (hoặc U0T/U0R)**

| Ký hiệu trên board | GPIO | Chức năng | Nối đến | Màu dây |
|-------------------|------|-----------|---------|----------|
| **UOT** (hoặc U0T) | GPIO1 | TX | ESP32 Controller G16 (RX) | Vàng |
| **UOR** (hoặc U0R) | GPIO3 | RX | ESP32 Controller G17 (TX) | Xanh lá |
| 5V | - | Power | Nguồn 5V (+) | Đỏ |
| GND | - | Ground | GND chung | Đen |

---

## 📸 HƯỚNG DẪN TỪ BƯỚC

### BƯỚC 1: NỐI ESP32-CAM VỚI ESP32 CONTROLLER

**⚠️ Ký hiệu trên board ESP32-CAM:**
- **UOT** (hoặc U0T) = TX (GPIO1)
- **UOR** (hoặc U0R) = RX (GPIO3)

```
⚠️ QUAN TRỌNG: TX NỐI VỚI RX (CHÉO NHAU)

ESP32 Controller                     ESP32-CAM (ký hiệu trên board)
    ┌────────┐                       ┌────────┐
    │        │    Dây XANH LÁ        │        │
    │  G17   │═══════════════════════│  UOR   │  (TX → RX)
    │        │                       │        │
    │        │    Dây VÀNG           │        │
    │  G16   │═══════════════════════│  UOT   │  (RX ← TX)
    │        │                       │        │
    │        │    Dây ĐEN            │        │
    │  GND   │═══════════════════════│  GND   │  (Bắt buộc!)
    │        │                       │        │
    └────────┘                       └────────┘
```

**Lưu ý quan trọng:**
- GND PHẢI được nối chung giữa 2 board!
- Không nối TX-TX hoặc RX-RX (sẽ không giao tiếp được)
- **UOT → G16** (CAM TX đến Controller RX)
- **UOR ← G17** (CAM RX nhận từ Controller TX)

---

### BƯỚC 2: NỐI HC-SR04 (CẢM BIẾN KHOẢNG CÁCH)

```
HC-SR04                    ESP32 Controller
┌───────────┐
│  ○ ○ ○ ○  │
│ V T E G  │
│ C R C N  │
│ C I H D  │
│   G O    │
└───────────┘
    │ │ │ │
    │ │ │ └────────── GND ─────────────► GND
    │ │ └──────────── ECHO (output) ───► G19
    │ └────────────── TRIG (input) ────► G18
    └──────────────── VCC ─────────────► 5V (hoặc 3.3V)
```

**Kiểm tra:**
- VCC có thể dùng 5V hoặc 3.3V (tùy module)
- ECHO trả về tín hiệu 3.3V nên an toàn cho ESP32

---

### BƯỚC 3: NỐI 3 SERVO

```
Servo có 3 dây:
- Đỏ/Đỏ nâu: VCC (5V)
- Nâu/Đen: GND
- Cam/Vàng: Signal (PWM)

                        ┌─────────────────────┐
                        │   ESP32 Controller  │
                        └─────────┬───────────┘
                                  │
         ┌────────────────────────┼────────────────────────┐
         │                        │                        │
         ▼                        ▼                        ▼
   ┌──────────┐            ┌──────────┐            ┌──────────┐
   │ Servo Đỏ │            │Servo Xanh│            │ Servo Xám│
   │  Bin 1   │            │  Bin 2   │            │  Bin 3   │
   └────┬─────┘            └────┬─────┘            └────┬─────┘
        │                       │                       │
    Signal → G12            Signal → G26            Signal → G32
    VCC → 5V                VCC → 5V                VCC → 5V
    GND → GND               GND → GND               GND → GND
```

**⚠️ Nguồn cho Servo:**
- 3 servo cần ~1.5A khi hoạt động cùng lúc
- KHÔNG cấp nguồn servo từ pin 5V của ESP32!
- Nên dùng nguồn ngoài 5V/2A riêng cho servo
- GND của nguồn servo PHẢI nối chung với GND ESP32

```
Cách nối nguồn servo đúng:
                          
    Nguồn 5V/2A                ESP32 Controller
   ┌──────────┐               ┌──────────────┐
   │  (+) 5V  │───────────────│→ VCC Servos  │
   │          │               │              │
   │  (-) GND │───────┬───────│→ GND Servos  │
   └──────────┘       │       │              │
                      └───────│→ GND ESP32   │  ← GND chung!
                              └──────────────┘
```

---

### BƯỚC 4: NỐI LCD 16x2 I2C

```
LCD I2C (mặt sau)              ESP32 Controller
┌─────────────────┐
│  GND VCC SDA SCL│
│   │   │   │   │ │
└───┼───┼───┼───┼─┘
    │   │   │   │
    │   │   │   └─────────────► G22 (SCL)
    │   │   └─────────────────► G21 (SDA)
    │   └─────────────────────► 5V (hoặc VIN)
    └─────────────────────────► GND
```

**Kiểm tra địa chỉ I2C:**
- Mặc định: `0x27`
- Nếu không hoạt động, thử: `0x3F`
- Để xác định chính xác: chạy I2C Scanner sketch

---

### BƯỚC 5: NỐI LED TRẠNG THÁI (Tùy chọn)

```
ESP32 G2 ────────[LED(+)]────[220Ω]──────► GND
                   │
              LED sáng khi HIGH
```

---

## 🔋 SƠ ĐỒ NGUỒN TỔNG THỂ

```
                    ┌────────────────────────────────┐
                    │       NGUỒN 5V/3A CHÍNH        │
                    │          (Adapter)              │
                    └───────────┬────────────────────┘
                                │
            ┌───────────────────┼───────────────────┐
            │                   │                   │
            ▼                   ▼                   ▼
    ┌──────────────┐    ┌──────────────┐    ┌──────────────┐
    │ ESP32 DevKit │    │  ESP32-CAM   │    │   Servos x3  │
    │   via VIN    │    │   via 5V     │    │   via VCC    │
    └──────┬───────┘    └──────┬───────┘    └──────┬───────┘
           │                   │                   │
           └───────────────────┴───────────────────┘
                               │
                           GND CHUNG
                               │
                    ┌──────────┴──────────┐
                    │ Nguồn 5V/3A (-) GND │
                    └─────────────────────┘
```

**⚠️ LƯU Ý QUAN TRỌNG VỀ NGUỒN:**
1. **GND phải nối chung** tất cả thiết bị
2. ESP32-CAM cần **ít nhất 500mA** riêng
3. Servos cần **~500mA x 3 = 1.5A** khi hoạt động
4. Tổng hệ thống nên có nguồn **5V/3A**
5. **KHÔNG** cấp nguồn qua USB laptop cho toàn bộ hệ thống

---

## ✅ CHECKLIST TRƯỚC KHI CẤP NGUỒN

- [ ] TX (G17) nối với RX (G3) - CHÉO
- [ ] RX (G16) nối với TX (G1) - CHÉO
- [ ] GND chung giữa tất cả boards
- [ ] Servo signal đúng pins (G12, G26, G32)
- [ ] HC-SR04 TRIG=G18, ECHO=G19
- [ ] LCD SDA=G21, SCL=G22
- [ ] Nguồn 5V đủ công suất (≥2A)
- [ ] Không có dây chập/đoản mạch

---

## 🔧 KIỂM TRA SAU KHI NỐI

### Test 1: Kiểm tra Serial giữa 2 board

1. Upload code cho cả 2 board
2. Mở Serial Monitor của ESP32 Controller (115200 baud)
3. Gõ `PING` và Enter
4. Nếu nhận được `PONG` → OK ✅
5. Nếu không → kiểm tra lại TX-RX và GND

### Test 2: Kiểm tra HC-SR04

1. Đặt tay cách sensor 10-20cm
2. Serial Monitor hiện: `[DISTANCE] xxcm detected`
3. Nếu luôn hiện 0 hoặc giá trị lạ → kiểm tra dây

### Test 3: Kiểm tra Servo

1. Khởi động → servo về góc 0°
2. Khi có lệnh mở bin → servo quay 90°
3. Nếu servo rung hoặc không quay → kiểm tra nguồn

### Test 4: Kiểm tra LCD

1. Khởi động → LCD hiện "Cho rac vao..."
2. Nếu không hiện → kiểm tra địa chỉ I2C (0x27 hoặc 0x3F)
3. Nếu hiện ô vuông → chỉnh biến trở contrast ở mặt sau

---

## 🖼️ HÌNH ẢNH MINH HỌA

### Vị trí chân ESP32 DevKit 30-pin:

```
        ┌─────────────────────────────────────┐
        │           ┌──────────┐              │
        │           │   USB    │              │
        │           └──────────┘              │
        │  ┌─────────────────────────────┐    │
        │  │ EN  │  │  │  │  │  │  │ D23 │    │
3V3 ────┤  │ G36 │  │  │  │  │  │  │ D22 │ SCL  LCD
        │  │ G39 │  │  │  │  │  │  │ TX0 │    │
        │  │ G34 │  │  │  │  │  │  │ RX0 │    │
        │  │ G35 │  │  │  │  │  │  │ D21 │ SDA  LCD
Servo G─┤  │ G32 │  │  │  │  │  │  │ GND │    │
        │  │ G33 │  │  │  │  │  │  │ D19 │ ECHO
        │  │ G25 │  │  │  │  │  │  │ D18 │ TRIG
Servo X─┤  │ G26 │  │  │  │  │  │  │ D5  │    │
        │  │ G27 │  │  │  │  │  │  │ D17 │ TX→CAM
        │  │ G14 │  │  │  │  │  │  │ D16 │ RX←CAM
Servo Đ─┤  │ G12 │  │  │  │  │  │  │ D4  │    │
        │  │ GND │  │  │  │  │  │  │ D2  │ LED
        │  │ G13 │  │  │  │  │  │  │ D15 │    │
        │  │ VIN │───5V─────────────┤────│    │
        │  └─────────────────────────────┘    │
        └─────────────────────────────────────┘
```

### Vị trí chân ESP32-CAM:

```
        ┌─────────────────────────┐
        │      [CAMERA LENS]      │
        ├─────────────────────────┤
        │ 5V  ○          ○  GND   │  ← Nguồn
        │ GND ○          ○  IO33  │
        │ IO12○          ○  IO1   │  ← TX (→ ESP32 G16)
        │ IO13○          ○  IO3   │  ← RX (← ESP32 G17)
        │ IO15○          ○  IO0   │  ← Boot mode
        │ IO14○          ○  VCC   │
        │ IO2 ○          ○  GND   │
        │ IO4 ○          ○  IO16  │  ← Flash LED
        └─────────────────────────┘
```

---

## 🚀 SẴN SÀNG!

Sau khi hoàn thành các bước trên:
1. Upload code cho ESP32 Controller
2. Upload code cho ESP32-CAM (dùng MB Programmer)
3. Nối Serial giữa 2 board
4. Cấp nguồn 5V/3A
5. Đợi hệ thống khởi động (~5 giây)
6. LCD hiện "Cho rac vao..."
7. Đặt rác trước sensor → Hệ thống hoạt động!

**Chúc thành công! 🎉**
