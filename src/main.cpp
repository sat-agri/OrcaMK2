#include <Arduino.h>

#include "config.h"
#include "motors.h"
#include "pixhawk.h"

void setup() {
    Serial.begin(config::SerialBaud);  // USB monitor.
    Serial1.begin(config::SerialBaud); // Pixhawk MAVLink UART.

    // Portenta's built-in LEDs are active LOW.
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);

    for (int flash = 0; flash < 3; ++flash) {
        digitalWrite(LEDB, LOW);
        delay(500);
        digitalWrite(LEDB, HIGH);
        delay(500);
    }

    motors::begin(); // Configures pins and briefly pulses each motor.
    pixhawk::begin();
}

void loop() {
    pixhawk::update();
}
