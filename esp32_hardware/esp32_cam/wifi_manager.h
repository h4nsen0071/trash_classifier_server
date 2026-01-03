/**
 * @file wifi_manager.h
 * @brief WiFi Connection Manager Module
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

// WiFi status
enum WifiStatus {
    WIFI_STATUS_DISCONNECTED,
    WIFI_STATUS_CONNECTING,
    WIFI_STATUS_CONNECTED,
    WIFI_STATUS_FAILED
};

/**
 * Khởi tạo WiFi
 * @return true nếu kết nối thành công
 */
bool wifiManager_init();

/**
 * Kết nối WiFi (blocking)
 * @return true nếu kết nối thành công
 */
bool wifiManager_connect();

/**
 * Ngắt kết nối WiFi
 */
void wifiManager_disconnect();

/**
 * Kiểm tra trạng thái kết nối
 * @return true nếu đang kết nối
 */
bool wifiManager_isConnected();

/**
 * Lấy IP address
 * @return IP address string
 */
String wifiManager_getIP();

/**
 * Lấy RSSI (cường độ tín hiệu)
 * @return RSSI in dBm
 */
int wifiManager_getRSSI();

/**
 * Lấy MAC address
 * @return MAC address string
 */
String wifiManager_getMAC();

/**
 * Thử kết nối lại
 * @return true nếu kết nối thành công
 */
bool wifiManager_reconnect();

/**
 * Kiểm tra và duy trì kết nối (gọi trong loop)
 * Tự động reconnect nếu mất kết nối
 */
void wifiManager_maintain();

#endif // WIFI_MANAGER_H
