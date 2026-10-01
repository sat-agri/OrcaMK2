#pragma once

namespace motors {
void begin(); // Includes the startup motor pulses.
void stop();
void driveForward(int speedPwm);
void driveReverse(int speedPwm);
void pivotRight(int speedPwm);
void driveManual(int throttleUs, int steeringUs);
}
