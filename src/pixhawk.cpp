#include <MAVLink_common.h>

#include "config.h"
#include "motors.h"
#include "pixhawk.h"

namespace pixhawk {
namespace {
unsigned long lastHeartbeatMs = 0;
unsigned long lastMessageMs = 0;
bool motorsActive = false;
bool autonomousMode = true;

float accelerationSamples[config::ImuSampleCount] = {};
float accelerationTotal = 0;
int sampleIndex = 0;

void recoverFromCollision() {
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDB, LOW);

    // Blocking recovery retains the existing behaviour: RC and link checks pause.
    motors::stop();
    delay(config::RecoveryStepMs);
    motors::driveReverse(config::RecoverySpeedPwm);
    delay(config::RecoveryStepMs);
    motors::stop();
    delay(config::RecoveryStepMs);
    motors::pivotRight(config::RecoverySpeedPwm);
    delay(config::RecoveryStepMs);
    motors::stop();
    delay(config::RecoveryStepMs);
    motors::driveForward(config::AutoSpeedPwm);

    // Discard commands queued during recovery and restart the collision filter.
    while (Serial1.available() > 0) Serial1.read();
    for (float& sample : accelerationSamples) sample = 0;
    accelerationTotal = 0;
    sampleIndex = 0;
    lastMessageMs = millis();
    digitalWrite(LEDB, HIGH);
}

void sendHeartbeat() {
    const unsigned long now = millis();
    if (now - lastHeartbeatMs < config::HeartbeatIntervalMs) return;
    lastHeartbeatMs = now;

    mavlink_message_t message;
    uint8_t bytes[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_heartbeat_pack(1, 191, &message, MAV_TYPE_ONBOARD_CONTROLLER,
        MAV_AUTOPILOT_INVALID, MAV_MODE_FLAG_SAFETY_ARMED, 0, MAV_STATE_ACTIVE);
    const uint16_t length = mavlink_msg_to_send_buffer(bytes, &message);
    Serial1.write(bytes, length);
}

void checkConnection() {
    if (millis() - lastMessageMs >= config::LinkTimeoutMs) {
        digitalWrite(LEDG, HIGH);
        digitalWrite(LEDB, HIGH);
        digitalWrite(LEDR, LOW);
        if (motorsActive || !autonomousMode) {
            motors::stop();
            motorsActive = false;
        }
    } else if (autonomousMode) {
        digitalWrite(LEDR, HIGH);
        digitalWrite(LEDB, HIGH);
        digitalWrite(LEDG, LOW);
        if (!motorsActive) {
            motors::driveForward(config::AutoSpeedPwm);
            motorsActive = true;
        }
    }
}

void handleRc(const mavlink_message_t& message) {
    mavlink_rc_channels_t channels;
    mavlink_msg_rc_channels_decode(&message, &channels);
    const bool nextAutonomousMode = channels.chan5_raw <= config::ModeThresholdUs;
    if (nextAutonomousMode != autonomousMode) {
        motors::stop();
        motorsActive = false;
        autonomousMode = nextAutonomousMode;
    }

    if (!autonomousMode) {
        digitalWrite(LEDR, HIGH);
        digitalWrite(LEDG, HIGH);
        digitalWrite(LEDB, LOW);
        motors::driveManual(channels.chan2_raw, channels.chan1_raw);
    }
}

bool handleImu(const mavlink_message_t& message) {
    mavlink_scaled_imu2_t imu;
    mavlink_msg_scaled_imu2_decode(&message, &imu);
    const float acceleration = imu.xacc;
    accelerationTotal -= accelerationSamples[sampleIndex];
    accelerationSamples[sampleIndex] = acceleration;
    accelerationTotal += acceleration;
    sampleIndex = (sampleIndex + 1) % config::ImuSampleCount;

    const float average = accelerationTotal / config::ImuSampleCount;
    if (abs(acceleration - average) > config::CollisionThresholdMg) {
        recoverFromCollision();
        return true;
    }
    return false;
}

void readMessages() {
    mavlink_message_t message;
    mavlink_status_t status;
    while (Serial1.available() > 0) {
        const uint8_t byte = Serial1.read();
        if (!mavlink_parse_char(MAVLINK_COMM_0, byte, &message, &status)) continue;
        lastMessageMs = millis();

        if (message.msgid == MAVLINK_MSG_ID_RC_CHANNELS) handleRc(message);
        if (message.msgid == MAVLINK_MSG_ID_SCALED_IMU2 && autonomousMode) {
            if (handleImu(message)) break;
        }
    }
}
}

void begin() {
    lastMessageMs = millis();
}

void update() {
    sendHeartbeat();
    checkConnection();
    readMessages();
}
}
