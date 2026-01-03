/**
 * @file distance_sensor.h
 * @brief HC-SR04 Distance Sensor Module
 */

#ifndef DISTANCE_SENSOR_H
#define DISTANCE_SENSOR_H

#include <Arduino.h>

/**
 * Khởi tạo sensor
 */
void distanceSensor_init();

/**
 * Đọc khoảng cách
 * @return Khoảng cách (cm), -1 nếu timeout/lỗi
 */
int distanceSensor_read();

/**
 * Kiểm tra có vật thể trong vùng phát hiện không
 * @return true nếu có vật thể
 */
bool distanceSensor_objectDetected();

/**
 * Kiểm tra vùng đã clear chưa
 * @return true nếu không còn vật thể
 */
bool distanceSensor_isCleared();

#endif // DISTANCE_SENSOR_H
