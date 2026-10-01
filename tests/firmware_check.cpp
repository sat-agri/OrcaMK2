#include <Arduino.h>
#include <MAVLink_common.h>
#include <cassert>
#include <iostream>

unsigned long clockMs = 0;
int digitalPins[64] = {};
int pwmPins[64] = {};
SerialPort Serial, Serial1;
void setup();
void loop();

void receive(const mavlink_message_t& message) {
    uint8_t bytes[MAVLINK_MAX_PACKET_LEN];
    const auto length = mavlink_msg_to_send_buffer(bytes, &message);
    Serial1.incoming.insert(Serial1.incoming.end(), bytes, bytes + length);
}

void rc(uint16_t throttle, uint16_t steering, uint16_t mode = 2000) {
    mavlink_rc_channels_t channels = {};
    channels.chancount = 8;
    channels.chan1_raw = steering;
    channels.chan2_raw = throttle;
    channels.chan5_raw = mode;
    mavlink_message_t message;
    mavlink_msg_rc_channels_encode(1, 1, &message, &channels);
    receive(message);
    loop();
}

void expectMotors(int left, int right) {
    assert(pwmPins[D6] == std::abs(left));
    assert(pwmPins[D5] == std::abs(right));
    if (left != 0) assert(digitalPins[A3] == (left > 0 ? HIGH : LOW));
    if (right != 0) assert(digitalPins[A4] == (right > 0 ? HIGH : LOW));
}

int main() {
    setup();
    expectMotors(0, 0);
    loop();
    expectMotors(200, 200);

    // Exercise real MAVLink decoding, RC mixing, deadband and saturation.
    rc(1500, 1500); expectMotors(0, 0);
    rc(2000, 1500); expectMotors(255, 255);
    rc(1000, 1500); expectMotors(-255, -255);
    rc(1500, 2000); expectMotors(255, -255);
    rc(1500, 1000); expectMotors(-255, 255);
    rc(2000, 2000); expectMotors(255, 0);
    rc(1510, 1490); expectMotors(0, 0);
    rc(1750, 1500); expectMotors(127, 127);

    clockMs += 1999;
    loop();
    expectMotors(127, 127);
    ++clockMs;
    loop();
    expectMotors(0, 0);
    assert(digitalPins[LEDR] == LOW);

    rc(1500, 1500);
    assert(digitalPins[LEDB] == LOW);
    rc(1500, 1500, 1000);
    expectMotors(0, 0); // Mode changes stop before Auto resumes next loop.
    loop();
    expectMotors(200, 200);

    mavlink_scaled_imu2_t imu = {};
    imu.xacc = 1000;
    mavlink_message_t message;
    mavlink_msg_scaled_imu2_encode(1, 1, &message, &imu);
    receive(message);
    const auto beforeRecovery = clockMs;
    loop();
    assert(clockMs - beforeRecovery >= 10000);
    expectMotors(200, 200);
    clockMs += 2000;
    loop();
    expectMotors(0, 0);

    // The outgoing stream must include a valid onboard-controller heartbeat.
    bool heartbeatSeen = false;
    mavlink_status_t status = {};
    for (const auto byte : Serial1.outgoing) {
        if (mavlink_parse_char(MAVLINK_COMM_1, byte, &message, &status) &&
            message.msgid == MAVLINK_MSG_ID_HEARTBEAT) {
            mavlink_heartbeat_t heartbeat;
            mavlink_msg_heartbeat_decode(&message, &heartbeat);
            assert(heartbeat.type == MAV_TYPE_ONBOARD_CONTROLLER);
            heartbeatSeen = true;
        }
    }
    assert(heartbeatSeen);
    std::cout << "Firmware checks passed\n";
}
