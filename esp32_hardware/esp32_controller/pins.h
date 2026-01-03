/**
 * @file pins.h
 * @brief GPIO Pin definitions
 */

#ifndef PINS_H
#define PINS_H

// Serial to CAM
#define PIN_SERIAL_RX       16
#define PIN_SERIAL_TX       17

// HC-SR04
#define PIN_TRIG            18
#define PIN_ECHO            19

// Servos
#define PIN_SERVO_RED       12      // Bin 1
#define PIN_SERVO_GREEN     26      // Bin 2
#define PIN_SERVO_GRAY      32      // Bin 3
#define PIN_SERVO_BIN1      PIN_SERVO_RED
#define PIN_SERVO_BIN2      PIN_SERVO_GREEN
#define PIN_SERVO_BIN3      PIN_SERVO_GRAY

// LCD I2C
#define PIN_LCD_SDA         21
#define PIN_LCD_SCL         22
#define LCD_ADDRESS         0x27
#define LCD_COLS            16
#define LCD_ROWS            2

// LED
#define PIN_LED_STATUS      2

#endif // PINS_H
