# BluetoothESP32andAPP

## Overview
This project demonstrates the integration between an ESP32 microcontroller and an Android application using Bluetooth. The repository contains the necessary code for both the ESP32 and the Android app, enabling seamless communication between the two devices.

## Features
- **ESP32 Bluetooth Server:** The ESP32 acts as a Bluetooth server, ready to connect with the Android app.
- **Android Application:** The app sends data to the ESP32 via Bluetooth.
- **PlatformIO Compatibility:** The ESP32 code is developed in PlatformIO but is also compatible with the Arduino IDE.

## Prerequisites
- **Hardware:** 
  - ESP32 microcontroller
  - Android device with Bluetooth capability
- **Software:** 
  - PlatformIO or Arduino IDE for ESP32
  - Android Studio for Android app development

## Getting Started

### 1. ESP32 Setup
1. Clone this repository and navigate to the `bluetooth_esp32` folder.
2. Open the project in PlatformIO or Arduino IDE.
3. Upload the code to your ESP32.

### 2. Android Application Setup
1. Navigate to the `android_app` directory.
2. Open the project in Android Studio.
3. Make sure Bluetooth permissions are properly set in the app's manifest file.
4. Build and run the app on your Android device.

### 3. Connecting and Using
1. Power on the ESP32 and open the Android app.
2. Search for the ESP32 device via Bluetooth on your phone.
3. Establish the connection and start sending data from the app to the ESP32.

## Troubleshooting
- **Connection Issues:** Double-check that Bluetooth permissions are enabled in the Android app.
- **Data Transmission Problems:** Ensure the correct UUIDs are being used in both the Android app and the ESP32 code.

## Contributions
Contributions are welcome! Feel free to submit pull requests or report issues to help improve the project.
