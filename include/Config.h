#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------------- LCD ----------------
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS    2

// ---------------- Encoder ----------------
#define ENCODER_CLK 2
#define ENCODER_DT  3
#define ENCODER_SW  4

// ---------------- Manual Limits ----------------
#define MIN_TEMP 30
#define MAX_TEMP 80

#define MIN_WEIGHT 100
#define MAX_WEIGHT 10000

#endif