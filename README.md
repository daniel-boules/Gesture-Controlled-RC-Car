# Gesture-Controlled RC Car

A wireless, gesture-controlled robotic car system built using an **ESP32** transmitter (worn on the hand) and an **ESP8266** receiver (mounted on the car). Communication is handled via the low-latency **ESP-NOW** protocol.

## System Overview

The project is divided into two main components:

### 1. Hand Controller (Transmitter)
- **Microcontroller**: ESP32
- **Sensor**: MPU6050 (Accelerometer + Gyroscope) via I2C (GPIO 4, 5)
- **Functions**: Utilizes FreeRTOS to run continuous sensor fusion, calculating X/Y tilt angles. The calculated forward and steering vectors are packed into a struct and transmitted wirelessly to the car via ESP-NOW.

### 2. RC Car (Receiver)
- **Microcontroller**: ESP8266
- **Actuators**: Two DC motors controlled via PWM
  - Speed pins: D1, D2
  - Direction pins: D3, D4, D5, D6
- **Sensors**: Two wheel encoders
  - Interrupt pins: D7, D8
  - Used for real-time RPM calculation and potential PID speed control
- **Functions**: Listens for incoming ESP-NOW messages and directly translates the hand tilt angles into differential steering and forward/reverse motor PWM commands.

## Setup Instructions

### Hardware Requirements
- 1x ESP32 Development Board
- 1x ESP8266 (NodeMCU/Wemos D1 Mini)
- 1x MPU6050 Breakout Board
- 1x Motor Driver (e.g., L298N or TB6612FNG)
- 2x DC Motors with Encoders
- Power source (Batteries/Regulators)

### Software Requirements
You will need the Arduino IDE with the ESP32 and ESP8266 board managers installed, along with the following libraries:
- `esp_now.h`
- `MPU6050.h` by Electronic Cats or similar

### Uploading
1. Upload `car_receiver.ino` from `car_receiver` to the ESP8266 first.
2. Open the Serial Monitor for the ESP8266 and print or record its Wi-Fi MAC address.
3. Edit `glove_transmitter.ino` in `glove_transmitter` and set `broadcastAddress` to that exact ESP8266 MAC.
4. Upload `glove_transmitter.ino` to the ESP32.
5. Power both devices and confirm ESP-NOW packet delivery in Serial output.

### ESP-NOW Pairing Notes
- Transmitter (`ESP32`) sends control data to the receiver MAC in `broadcastAddress`.
- Receiver (`ESP8266`) sends encoder feedback to its configured peer address.
- If no motion occurs, verify MAC addresses, board power, and matching baud rate (115200).

## Authors

**Team 26** — Developed as a Mechatronics/Robotics hardware integration project.

## License

MIT License
