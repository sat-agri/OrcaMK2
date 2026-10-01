//Configs.h
#pragma once //Prevents files from being included multiple times
#include <Arduino.h>

// ==================== PIN DEFINITIONS ====================
// Motor 1 (Left Side)
constexpr byte PWM_MOTOR_1 = D6;  
constexpr byte DIR_MOTOR_1 = A3;  

// Motor 2 (Right Side)
constexpr byte PWM_MOTOR_2 = D5;  
constexpr byte DIR_MOTOR_2 = A4;  

// ==================== TUNING PARAMETERS ====================
// IMU Collision Detection (Moving Average Filter)
constexpr int SAMPLE_RANGE = 10;           // Number of samples for moving average
constexpr float CRASH_THRESHOLD = 200.0;   // Acceleration spike limit to trigger escape sequence