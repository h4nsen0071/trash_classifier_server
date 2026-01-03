/**
 * @file http_client.h
 * @brief HTTP Client Module for Server Communication
 */

#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <Arduino.h>

// HTTP Response structure
struct HttpResponse {
    bool success;
    int statusCode;
    String body;
    String error;
};

// Classification result structure
struct ClassificationResult {
    bool success;
    String className;      // "paper", "plastic", "glass"
    int binNumber;         // 1, 2, 3
    float confidence;
    String error;
};

/**
 * Khởi tạo HTTP client
 */
void httpClient_init();

/**
 * Gửi ảnh lên server để phân loại
 * @param imageBase64 Ảnh đã encode base64
 * @return ClassificationResult
 */
ClassificationResult httpClient_classify(const String& imageBase64);

/**
 * Kiểm tra kết nối với server
 * @return true nếu server có thể truy cập
 */
bool httpClient_ping();

/**
 * Lấy thời gian request cuối cùng
 * @return Thời gian (ms)
 */
unsigned long httpClient_getLastRequestTime();

#endif // HTTP_CLIENT_H
