# OrcaMK2
OrcaMK2 controls a two-wheel skid-steer robotic hull communicating with a Pixhawk via MAVLink Protocol. Supports autonomous navigation with IMU-based collision detection (Escape Sequence) and manual RC operation. Includes heartbeat monitoring and failsafe mechanisms to halt motors if the Pixhawk connection drops.

# RobotDuck --> OrcaMK1_DualDrive Firmware

**Author:** Nazrin Hakeem Bin Khalid

**Date:** August 2026

**Status:** [Needs more field testing]

## 📝 Overview
This project controls a two-wheel skid-steer robotic body using an Arduino Portenta H7. It communicates with a Holybro Pixhawk 6X via the MAVLink Protocol. 

The firmware supports two modes:
1.  **Autonomous Mode:** Drives forward automatically. Uses moving-average IMU data to detect collisions and execute a programmed escape sequence.
2.  **Manual Mode:** Direct skid-steer control via RC controller and transmitter.

It also includes built-in failsafes to immediately halt all motors if communication with the Pixhawk is lost.

---

## 🛠️ Hardware & Wiring
*   **Microcontroller:** Arduino Portenta H7 + Portenta Breakout Board
*   **Flight Controller:** Holybro Pixhawk 6X
*   **Motor Driver:** Cytron MDDS30 SmartDriveDup

### Pin Connections
All pin configurations are defined in `Configs.h`. 
*   **Motor 1 (Left):** PWM = D6, DIR = A3
*   **Motor 2 (Right):** PWM = D5, DIR = A4
*   **Pixhawk UART:** Serial1 (57600 baud)

*   Here is the link of the wiring diagram: https://app.cirkitdesigner.com/project/8dd8c75b-f0f1-4eca-acd8-ed756c08ec5a

---

## 💻 Software & Dependencies
*   **IDE:** Arduino IDE 2.3.8 (or newer)
*   **Required Libraries:**
    *   `MAVLink_common.h` - Must be installed in your Arduino libraries folder. Version: [MAVLink by Oleg Kalachev (2.0.29 or newer)]

---

## 📂 Project Structure
This codebase is modular. **Do not put all code in one file.**

*   `OrcaMK1_DualDrive.ino`: The main loop, setup, and global variables.
*   `Configs.h`: **Start Here.** This is the control panel. Change pin assignments, crash detection thresholds, and sample rates here.
*   `Motors.ino`: Skid-steer mixing math and basic movement functions (forward, reverse, pivot).
*   `MAVLink_Logic.ino`: Handles heartbeat, RC parsing (CH5 mode switch), IMU crash detection, and the failsafe system.

---

## 🚀 Operation Guide

### Startup Sequence
1. Power on the Pixhawk and Portenta.
2. The Portenta will flash its **BLUE LED** three times.
3. The motors will briefly pulse (Pre-flight warmup) to confirm driver connection.

### Status LEDs (Portenta Built-in)
*   🟢 **Solid GREEN:** Autonomous Mode Active.
*   🔵 **Solid BLUE:** Manual RC Mode Active.
*   🟦 **Cyan (Green + Blue):** Escape Sequence triggered (Collision detected).
*   🔴 **Solid RED:** CRITICAL FAILSAFE. Pixhawk connection lost. Motors halted.

### RC Control Mapping
*   **CH5 (Mode Switch):** UP = Auto Mode, DOWN = Manual Mode.
*   **CH1 (Aileron):** Steering (Manual Mode).
*   **CH2 (Elevator):** Forward/Reverse (Manual Mode).

---

## ⚠️ Known Quirks & Next Steps
*   Will need further field testing
