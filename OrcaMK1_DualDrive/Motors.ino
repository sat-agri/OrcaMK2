// ==========================================
// MOTOR CONTROL FUNCTIONS
// ==========================================
void driveForward(int speed) {
  digitalWrite(DIR_MOTOR_1, HIGH);
  digitalWrite(DIR_MOTOR_2, HIGH);
  delay(50); 
  analogWrite(PWM_MOTOR_2, speed); 
  delay(200);
  analogWrite(PWM_MOTOR_1, speed);
}

void driveReverse(int speed) {
  digitalWrite(DIR_MOTOR_1, LOW);
  digitalWrite(DIR_MOTOR_2, LOW);
  delay(50);
  analogWrite(PWM_MOTOR_2, speed);
  delay(200);
  analogWrite(PWM_MOTOR_1, speed);
}

void pivotRight(int speed) {
  digitalWrite(DIR_MOTOR_1, LOW); 
  digitalWrite(DIR_MOTOR_2, HIGH);  
  delay(50);
  analogWrite(PWM_MOTOR_2, speed);
  delay(200);
  analogWrite(PWM_MOTOR_1, speed);
}

void stopMotors() {
  analogWrite(PWM_MOTOR_1, 0);
  analogWrite(PWM_MOTOR_2, 0);
  delay(50);
  digitalWrite(DIR_MOTOR_1, LOW); 
  digitalWrite(DIR_MOTOR_2, LOW);
}

// ==========================================
// SKID STEER MANUAL MIXER
// ==========================================
void driveManual(int forwardRaw, int steerRaw) {
  // Map standard RC PWM (1000-2000µs) to 8-bit Motor PWM (-255 to +255)
  // 1500 is the center resting position of the joystick
  int forward = map(forwardRaw, 1000, 2000, -255, 255);
  int steer = map(steerRaw, 1000, 2000, -255, 255);

  // DEADBAND: Ignore tiny stick movements near the center to prevent drift
  if (abs(forward) < 30) forward = 0;
  if (abs(steer) < 30) steer = 0;

  // Skid Steer Mixing Math
  // Forward stick drives both forward; Steering stick offsets them
  int leftSpeed = forward + steer;
  int rightSpeed = forward - steer;

  // Constrain to maximum hardware limits to prevent overflow
  leftSpeed = constrain(leftSpeed, -255, 255);
  rightSpeed = constrain(rightSpeed, -255, 255);

  // Execute Left Motor (Motor 1)
  if (leftSpeed >= 0) {
    digitalWrite(DIR_MOTOR_1, HIGH);
    analogWrite(PWM_MOTOR_1, leftSpeed);
  } else {
    digitalWrite(DIR_MOTOR_1, LOW);
    analogWrite(PWM_MOTOR_1, abs(leftSpeed));
  }

  // Execute Right Motor (Motor 2)
  if (rightSpeed >= 0) {
    digitalWrite(DIR_MOTOR_2, HIGH);
    analogWrite(PWM_MOTOR_2, rightSpeed);
  } else {
    digitalWrite(DIR_MOTOR_2, LOW);
    analogWrite(PWM_MOTOR_2, abs(rightSpeed));
  }
}