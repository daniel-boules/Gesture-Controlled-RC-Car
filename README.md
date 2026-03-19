# Gesture-Controlled RC Car

![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![Platform: ESP32 / ESP8266](https://img.shields.io/badge/Platform-ESP32%20%2F%20ESP8688-blue.svg)
![Protocol: ESP-NOW](https://img.shields.io/badge/Protocol-ESP--NOW-green.svg)

A wireless, gesture-controlled robotic car system built using an **ESP32** transmitter (worn on the hand) and an **ESP8266** receiver (mounted on the car). Communication is handled via the low-latency **ESP-NOW** protocol.

---

## Table of Contents

1. [Overview](#overview)
2. [System Architecture](#system-architecture)
3. [Repository Structure](#repository-structure)
4. [Getting Started](#getting-started)
5. [ESP-NOW Pairing Notes](#esp-now-pairing-notes)
6. [Authors](#authors)
7. [Acknowledgments](#acknowledgments)
8. [License](#license)

---

## Overview

| Property | Value |
|----------|-------|
| Transmitter MCU | ESP32 |
| Receiver MCU | ESP8266 (NodeMCU / Wemos D1 Mini) |
| IMU | MPU6050 (Accelerometer + Gyroscope, I2C) |
| Communication | ESP-NOW (peer-to-peer, ~1 ms latency) |
| Motor Control | PWM differential drive via motor driver |
| RTOS | FreeRTOS (sensor fusion task on ESP32) |

---

## System Architecture

### 1. Hand Controller — Glove Transmitter (ESP32)

| Component | Detail |
|-----------|--------|
| MCU | ESP32 |
| Sensor | MPU6050 via I2C (SDA: GPIO 4, SCL: GPIO 5) |
| Logic | FreeRTOS task for continuous sensor fusion; X/Y tilt angles packed into a struct and sent via ESP-NOW |

### 2. RC Car — Receiver (ESP8266)

| Component | Detail |
|-----------|--------|
| MCU | ESP8266 |
| Motors | 2× DC motors — speed pins D1, D2; direction pins D3–D6 |
| Encoders | 2× wheel encoders on interrupt pins D7, D8 — real-time RPM calculation |
| Logic | Translates received tilt angles into differential steering and forward/reverse PWM commands |

---

## Repository Structure

```
.
├── car_receiver/
│   └── car_receiver.ino       # ESP8266 receiver sketch
├── glove_transmitter/
│   └── glove_transmitter.ino  # ESP32 transmitter sketch
├── .gitignore
├── LICENSE
└── README.md
```

---

## Getting Started

### Hardware Requirements

- 1× ESP32 Development Board
- 1× ESP8266 (NodeMCU / Wemos D1 Mini)
- 1× MPU6050 Breakout Board
- 1× Motor Driver (L298N or TB6612FNG)
- 2× DC Motors with Encoders
- Power source (batteries + regulators)

### Software Requirements

- Arduino IDE with ESP32 and ESP8266 board managers installed
- Libraries:
  - `esp_now.h` (bundled with ESP32/ESP8266 board package)
  - `MPU6050.h` (Electronic Cats or similar)

### Upload Sequence

1. Upload `car_receiver.ino` to the **ESP8266** first.
2. Open Serial Monitor (115200 baud) on the ESP8266 and record its Wi-Fi MAC address.
3. Edit `glove_transmitter.ino` — set `broadcastAddress[]` to the exact ESP8266 MAC.
4. Upload `glove_transmitter.ino` to the **ESP32**.
5. Power both devices and confirm ESP-NOW packet delivery in Serial output.

---

## ESP-NOW Pairing Notes

- The ESP32 transmitter sends control data to the receiver MAC stored in `broadcastAddress`.
- The ESP8266 receiver sends encoder feedback back to its configured peer address.
- If no motion occurs: verify MAC addresses, board power, and matching baud rate (115200).

---

## Authors

| Name | Affiliation |
|------|-------------|
| Daniel Boules | Mechatronics Engineering, German University in Cairo |
| Andrew Abdelmalak | Mechatronics Engineering, German University in Cairo |
| Kirolous Magdy | Mechatronics Engineering, German University in Cairo |
| Youssef Youssry | Mechatronics Engineering, German University in Cairo |

---

## Acknowledgments

Developed as part of the **Robotics** course (Team 26) at the German University in Cairo, Faculty of Engineering and Material Science.

---

## License

Distributed under the MIT License. See [LICENSE](LICENSE) for details.
