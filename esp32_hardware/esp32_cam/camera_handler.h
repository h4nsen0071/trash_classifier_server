/**
 * @file camera_handler.h
 * @brief Camera Operations Module
 */

#ifndef CAMERA_HANDLER_H
#define CAMERA_HANDLER_H

#include <Arduino.h>
#include "esp_camera.h"

/**
 * Khởi tạo camera
 * @return true nếu thành công
 */
bool cameraHandler_init();

/**
 * Chụp ảnh và trả về framebuffer
 * @return camera_fb_t* pointer, NULL nếu lỗi
 * 
 * LƯU Ý: Phải gọi cameraHandler_releaseFrame() sau khi dùng xong!
 */
camera_fb_t* cameraHandler_capture();

/**
 * Giải phóng framebuffer sau khi sử dụng
 * @param fb Framebuffer cần giải phóng
 */
void cameraHandler_releaseFrame(camera_fb_t* fb);

/**
 * Chụp ảnh và encode thành base64
 * @param base64Output String để chứa output
 * @return true nếu thành công
 */
bool cameraHandler_captureToBase64(String& base64Output);

/**
 * Bật Flash LED
 */
void cameraHandler_flashOn();

/**
 * Tắt Flash LED
 */
void cameraHandler_flashOff();

/**
 * Lấy kích thước ảnh cuối cùng
 * @return Kích thước bytes
 */
size_t cameraHandler_getLastImageSize();

#endif // CAMERA_HANDLER_H
