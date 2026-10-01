#include "motors.h"
#include "config.h"

namespace motors {
namespace {
// Automatic manoeuvres retain the hardware's direction-settle and staggered start.
void driveWithDelay(int leftPwm, int rightPwm) {
    digitalWrite(config::LeftDirectionPin, leftPwm >= 0 ? HIGH : LOW);
    digitalWrite(config::RightDirectionPin, rightPwm >= 0 ? HIGH : LOW);
    delay(config::DirectionSettleMs);
    analogWrite(config::RightPwmPin, abs(rightPwm));
    delay(config::RightMotorLeadMs);
    analogWrite(config::LeftPwmPin, abs(leftPwm));
}
}

void begin() {
    pinMode(config::LeftPwmPin, OUTPUT);
    pinMode(config::LeftDirectionPin, OUTPUT);
    pinMode(config::RightPwmPin, OUTPUT);
    pinMode(config::RightDirectionPin, OUTPUT);

    digitalWrite(config::LeftDirectionPin, HIGH);
    digitalWrite(config::RightDirectionPin, HIGH);
    delay(100);
    analogWrite(config::LeftPwmPin, config::AutoSpeedPwm);
    delay(150);
    analogWrite(config::LeftPwmPin, 0);
    delay(1000);
    analogWrite(config::RightPwmPin, config::AutoSpeedPwm);
    delay(150);
    analogWrite(config::RightPwmPin, 0);
    delay(1000);
}

void stop() {
    analogWrite(config::LeftPwmPin, 0);
    analogWrite(config::RightPwmPin, 0);
    delay(config::DirectionSettleMs);
    digitalWrite(config::LeftDirectionPin, LOW);
    digitalWrite(config::RightDirectionPin, LOW);
}

void driveForward(int speedPwm) { driveWithDelay(speedPwm, speedPwm); }
void driveReverse(int speedPwm) { driveWithDelay(-speedPwm, -speedPwm); }
void pivotRight(int speedPwm) { driveWithDelay(-speedPwm, speedPwm); }

void driveManual(int throttleUs, int steeringUs) {
    int throttle = map(throttleUs, config::RcMinimumUs, config::RcMaximumUs, -255, 255);
    int steering = map(steeringUs, config::RcMinimumUs, config::RcMaximumUs, -255, 255);
    if (abs(throttle) < config::ManualDeadbandPwm) throttle = 0;
    if (abs(steering) < config::ManualDeadbandPwm) steering = 0;

    const int leftPwm = constrain(throttle + steering, -255, 255);
    const int rightPwm = constrain(throttle - steering, -255, 255);
    digitalWrite(config::LeftDirectionPin, leftPwm >= 0 ? HIGH : LOW);
    analogWrite(config::LeftPwmPin, abs(leftPwm));
    digitalWrite(config::RightDirectionPin, rightPwm >= 0 ? HIGH : LOW);
    analogWrite(config::RightPwmPin, abs(rightPwm));
}
}
