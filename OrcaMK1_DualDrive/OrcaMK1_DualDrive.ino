/*************************************************************************************************
 * Project Name:  OrcaMK1_DualDrive
 * Author:        Nazrin Hakeem Bin Khalid - Tech Intern
 * Handover Date: August 2026
 * Microcontroller: Arduino Portenta H7 + Portenta Breakout Board
 * IDE Version:   Arduino IDE 2.3.8
 * Hardware:      Holybro Pixhawk 6X, Cytron MDDS30 SmartDriveDup Motor Driver
 * 
 * Overview: 
 * Controls a two-wheel skid-steer robotic hull communicating with a Pixhawk 
 * via MAVLink Protocol. Supports autonomous navigation with IMU-based collision 
 * detection (Escape Sequence) and manual RC operation. Includes heartbeat monitoring 
 * and failsafe mechanisms to halt motors if the Pixhawk connection drops.
 *************************************************************************************************/

#include <MAVLink_common.h>
#include "Configs.h"

// ==================== SYSTEM STATE VARIABLES ====================
unsigned long lastHeartbeat = 0;
unsigned long lastPixhawkMessage = 0;

// IMU Tracking
float xacc_value[SAMPLE_RANGE] = {0};
int readIndex = 0;
float total_xacc = 0;
float average_xacc = 0;

// Mode Tracking
bool motorsActive = false; 
bool autonomousMode = true;     // Tracks the state of RC CH5
bool lastAutonomousMode = true; // Detects when the switch is flipped to ensure safe transition

void setup() {
  Serial.begin(57600);  // USB Debugging Serial
  Serial1.begin(57600); // UART connection to Pixhawk
  
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  
  // Portenta LEDs are active-LOW. Writing HIGH turns them OFF.
  digitalWrite(LEDR, HIGH); 
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

  // Startup Indicator: Flash BLUE three times
  for(int i = 0; i < 3; i++) {
    digitalWrite(LEDB, LOW);
    delay(500);
    digitalWrite(LEDB, HIGH);
    delay(500);
  }

  pinMode(PWM_MOTOR_1, OUTPUT);
  pinMode(DIR_MOTOR_1, OUTPUT);
  pinMode(PWM_MOTOR_2, OUTPUT);
  pinMode(DIR_MOTOR_2, OUTPUT);

  // ==========================================
  // PRE-FLIGHT WARMUP SEQUENCE
  // Briefly pulse motors to confirm connection
  // ==========================================
  digitalWrite(DIR_MOTOR_1, HIGH);
  digitalWrite(DIR_MOTOR_2, HIGH);
  delay(100);

  analogWrite(PWM_MOTOR_1, 200);
  delay(150);
  analogWrite(PWM_MOTOR_1, 0);
  delay(1000); 

  analogWrite(PWM_MOTOR_2, 200);
  delay(150);
  analogWrite(PWM_MOTOR_2, 0);
  delay(1000); 

  lastPixhawkMessage = millis();
}

// ==========================================
// MAIN LOOP
// ==========================================
void loop() {
  sendHeartbeat();
  monitorFailsafe();
  processMAVLink();
}