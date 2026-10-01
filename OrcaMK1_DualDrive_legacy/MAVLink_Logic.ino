// ==========================================
// AUTONOMOUS COLLISION RECOVERY
// ==========================================
void executeEscapeSequence() {
  // Set LED to CYAN (Green + Blue) to indicate escape mode
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, LOW); 
  
  stopMotors();    delay(2000); 
  driveReverse(150); delay(2000); 
  stopMotors();    delay(2000);
  pivotRight(150);   delay(2000); 
  stopMotors();    delay(2000);
  
  driveForward(200); 
  
  // Flush the serial buffer to clear out any old commands queued during the delay()s
  while(Serial1.available() > 0) { Serial1.read(); }

  // Reset the moving average filter so the robot doesn't immediately think it's still crashing
  for (int i = 0; i < SAMPLE_RANGE; i++) { xacc_value[i] = 0.0; }
  total_xacc = 0.0;
  readIndex = 0;
  
  lastPixhawkMessage = millis(); 
  digitalWrite(LEDB, HIGH); // Turn off blue, leaving Green (Auto Mode)
}

// ==========================================
// MAVLINK PARSING & HEALTH MONITORING
// ==========================================
void sendHeartbeat() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastHeartbeat >= 1000){
    lastHeartbeat = currentMillis;
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_heartbeat_pack(1, 191, &msg, MAV_TYPE_ONBOARD_CONTROLLER, MAV_AUTOPILOT_INVALID, MAV_MODE_FLAG_SAFETY_ARMED, 0, MAV_STATE_ACTIVE); 
    uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
    Serial1.write(buf, len);
  }
}

void processMAVLink() {
  mavlink_message_t msg;
  mavlink_status_t status;

  while (Serial1.available() > 0) {
    uint8_t c = Serial1.read();
    
    if (mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
      lastPixhawkMessage = millis(); 

      // --- 1. PARSE RC DATA (Switching Modes & Manual Driving) ---
      if (msg.msgid == MAVLINK_MSG_ID_RC_CHANNELS) {
        mavlink_rc_channels_t rc;
        mavlink_msg_rc_channels_decode(&msg, &rc);
        
        // CH5 controls mode: Up (<1500) = Auto, Down (>1500) = Manual
        if (rc.chan5_raw > 1500) {
          autonomousMode = false; 
        } else {
          autonomousMode = true;  
        }

        // Safe Transition: Stop motors instantly when the switch is flipped
        if (autonomousMode != lastAutonomousMode) {
          stopMotors();
          motorsActive = false;
          lastAutonomousMode = autonomousMode;
        }

        // If in Manual Mode, immediately pass Elevator and Aileron to the mixer
        if (!autonomousMode) {
          digitalWrite(LEDR, HIGH);
          digitalWrite(LEDG, HIGH);
          digitalWrite(LEDB, LOW); // Solid BLUE indicates Manual Mode
          
          driveManual(rc.chan2_raw, rc.chan1_raw);
        }
      }

      // --- 2. PARSE SENSOR DATA (Crash Detection in Auto Mode) ---
      if (msg.msgid == MAVLINK_MSG_ID_SCALED_IMU2 && autonomousMode) {
        mavlink_scaled_imu2_t scaled_imu2;
        mavlink_msg_scaled_imu2_decode(&msg, &scaled_imu2);
        
        float current_xacc = scaled_imu2.xacc;
        
        // Calculate the moving average of the X-axis acceleration
        total_xacc = total_xacc - xacc_value[readIndex];
        xacc_value[readIndex] = current_xacc;
        total_xacc = total_xacc + xacc_value[readIndex];
        
        readIndex = readIndex + 1;
        if (readIndex >= SAMPLE_RANGE) readIndex = 0;

        average_xacc = total_xacc / SAMPLE_RANGE;
        
        // If the current reading spikes drastically from the average, it's a collision
        float difference = abs(current_xacc - average_xacc);

        if (difference > CRASH_THRESHOLD) {
          executeEscapeSequence();
          break;
        }
      }
    }
  }
}

void monitorFailsafe() {
  unsigned long currentMillis = millis();
  
  // FAILSAFE: If Pixhawk dies or communication drops for 2 seconds, slam the brakes
  if (currentMillis - lastPixhawkMessage >= 2000) {
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);
    digitalWrite(LEDR, LOW); // Solid RED indicates Critical Failure
    
    if (motorsActive == true || !autonomousMode) {
      stopMotors();
      motorsActive = false; 
    }
  } else {
    // SYSTEM HEALTHY
    // Only manage automatic forward drive if we are in Autonomous mode
    if (autonomousMode) {
      digitalWrite(LEDR, HIGH);
      digitalWrite(LEDB, HIGH);
      digitalWrite(LEDG, LOW); // Solid GREEN indicates Auto Mode
      
      if (motorsActive == false) {
        driveForward(200);
        motorsActive = true; 
      }
    } 
  }
}