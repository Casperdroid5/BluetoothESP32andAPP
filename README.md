# Remote Controlled 6DOF Robotic Arm 

This project allows you to control a 6 Degrees of Freedom (6DOF) robotic arm using an ESP32 microcontroller via Bluetooth an a android APP. The robotic arm can be wirelessly controlled from a mobile application built with MIT App Inventor.

## Features

- **Bluetooth Control**: Control the robotic arm wirelessly using Bluetooth.
- **Servo Motor Control**: Move each of the 6 servos individually.
- **Save and Replay**: Save positions of the servos and replay them in a loop.
- **Pause and Resume**: Pause the replay of saved positions and resume when needed.
- **Reset Functionality**: Reset the saved positions and stop the replay at any time.

## Hardware Requirements

- ESP32 Development Board (with classic bluetooth support)
- 6, 180-degrees Servo Motors
- Bluetooth-enabled android smartphone or tablet
- Power Supply for Servo Motors

## Software Requirements

- [Arduino IDE](https://www.arduino.cc/en/software)
- [MIT App Inventor](http://appinventor.mit.edu/)

## Circuit Diagram

Connect the servo motors to the ESP32 as follows (but feel free to choose other PWM pins):

| Servo | ESP32 Pin |
|-------|-----------|
| S1    | GPIO 12   |
| S2    | GPIO 10   |
| S3    | GPIO 25   |
| S4    | GPIO 26   |
| S5    | GPIO 33   |
| S6    | GPIO 32   |

Additionally, connect an LED to GPIO 2 for indicating the connection status. This LED is buildin in the ESP32-WROOM-32 dev-module that was used for this project.

