/**
 * @file config.h
 * @brief CẤU HÌNH CHÍNH CHO ESP32-CAM
 * 
 * ⚙️ CHỈNH SỬA CÁC GIÁ TRỊ Ở ĐÂY ĐỂ THAY ĐỔI HÀNH VI HỆ THỐNG
 * Không cần sửa các file khác!
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// 📶 WIFI CONFIGURATION
// ============================================================

// Tên mạng WiFi (chỉ hỗ trợ 2.4GHz)
#define WIFI_SSID           "YOUR_WIFI_SSID"

// Mật khẩu WiFi
#define WIFI_PASSWORD       "YOUR_WIFI_PASSWORD"

// Timeout kết nối WiFi (ms)
#define WIFI_CONNECT_TIMEOUT_MS     20000

// Số lần thử kết nối lại khi mất WiFi
#define WIFI_RECONNECT_ATTEMPTS     3

// Delay giữa các lần thử (ms)
#define WIFI_RECONNECT_DELAY_MS     5000

// ============================================================
// 🌐 SERVER CONFIGURATION
// ============================================================

// URL endpoint phân loại
// Option 1: Production (Cloudflare Tunnel)
#define SERVER_URL          "https://api.smartbin.live/classify"

// Option 2: Local development (uncomment để dùng)
// #define SERVER_URL       "http://192.168.1.100:5000/classify"

// Timeout cho HTTP request (ms)
#define HTTP_TIMEOUT_MS             15000

// ============================================================
// 📷 CAMERA CONFIGURATION
// ============================================================

// Độ phân giải ảnh
// YOLO11 classification sử dụng input 224x224, nên chụp ảnh nhỏ để:
// - Upload nhanh hơn (5-10KB thay vì 50-80KB)
// - Server không cần resize nhiều
// - Response time nhanh hơn (~1s thay vì 2-3s)
//
// FRAMESIZE_240X240 = 240x240  ⭐ KHUYẾN NGHỊ (gần nhất với 224x224)
// FRAMESIZE_QVGA    = 320x240  (backup option)
// FRAMESIZE_VGA     = 640x480  (nếu cần độ chi tiết cao hơn)
// FRAMESIZE_SVGA    = 800x600  (chậm, file lớn)
#define CAMERA_FRAME_SIZE   FRAMESIZE_240X240

// Chất lượng JPEG (10-63, càng nhỏ càng nét nhưng file lớn hơn)
// Với 240x240, quality 10-12 cho file ~5-10KB
#define CAMERA_JPEG_QUALITY 10

// Độ sáng camera (-2 đến 2)
#define CAMERA_BRIGHTNESS   0

// Độ tương phản (-2 đến 2)
#define CAMERA_CONTRAST     0

// Độ bão hòa màu (-2 đến 2)
#define CAMERA_SATURATION   0

// Bật Flash LED khi chụp
#define USE_FLASH_LED       false

// ============================================================
// 🎯 CLASSIFICATION SETTINGS
// ============================================================

// Ngưỡng confidence tối thiểu (0.0 - 1.0)
// Nếu confidence < ngưỡng này → báo lỗi LOW_CONFIDENCE
#define MIN_CONFIDENCE      0.6

// ============================================================
// 🔌 SERIAL COMMUNICATION
// ============================================================

// Baud rate (PHẢI GIỐNG ESP32 Controller)
#define SERIAL_BAUD         115200

// ============================================================
// 🐛 DEBUG - Bật/Tắt logging
// ============================================================

// Bật logging chi tiết (comment để tắt)
#define DEBUG_ENABLED

// Log từng module riêng (comment để tắt module không cần)
#define DEBUG_WIFI
#define DEBUG_CAMERA
#define DEBUG_HTTP
#define DEBUG_SERIAL_COMM
#define DEBUG_JSON

// ============================================================
// 📊 SYSTEM INFO
// ============================================================

#define FIRMWARE_VERSION    "1.0.0"
#define DEVICE_NAME         "SmartBin-CAM"

#endif // CONFIG_H
