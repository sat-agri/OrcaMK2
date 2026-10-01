#pragma once

#include <Arduino.h>

// Hardware wiring and field-tunable values live here.
namespace config {
constexpr byte LeftPwmPin = D6;
constexpr byte LeftDirectionPin = A3;
constexpr byte RightPwmPin = D5;
constexpr byte RightDirectionPin = A4;

constexpr unsigned long SerialBaud = 57600;
constexpr unsigned long HeartbeatIntervalMs = 1000;
constexpr unsigned long LinkTimeoutMs = 2000;

// CH1 = steering, CH2 = throttle, CH5 = mode.
// CH5 above the threshold selects Manual; at or below selects Auto.
constexpr int RcMinimumUs = 1000;
constexpr int RcMaximumUs = 2000;
constexpr int ModeThresholdUs = 1500;
constexpr int ManualDeadbandPwm = 30;

// Motor commands use 8-bit PWM: 0 = stopped, 255 = full power.
constexpr int AutoSpeedPwm = 200;
constexpr int RecoverySpeedPwm = 150;
constexpr unsigned long DirectionSettleMs = 50;
constexpr unsigned long RightMotorLeadMs = 200;
constexpr unsigned long RecoveryStepMs = 2000;

// SCALED_IMU2 acceleration is reported in milli-g.
constexpr int ImuSampleCount = 10;
constexpr float CollisionThresholdMg = 200.0f;
}
