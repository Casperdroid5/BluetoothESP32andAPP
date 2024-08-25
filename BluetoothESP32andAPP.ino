#include <ESP32Servo.h>
#include "BluetoothSerial.h"

// Number of servo motors
const int numServos = 6;

// Servo objects and pin configurations
Servo servos[numServos];
const int servoPins[numServos] = {12, 10, 25, 26, 33, 32};

// BluetoothSerial object
BluetoothSerial SerialBT;

// LED pin
const int ledPin = 2;

void setup() {
  Serial.begin(115200);            // Start USB serial communication for debugging
  SerialBT.begin("ESP32_BT_6DOF-Arm");      // Start Bluetooth with the name ESP32_BT

  // Attach servos to their pins and set initial positions
  initializeServos();
  
  pinMode(ledPin, OUTPUT);         // Set LED pin as output
  Serial.println("ESP32 Ready");
}

void loop() {
  if (SerialBT.available()) {      // Check if data is available via Bluetooth
    String data = SerialBT.readStringUntil('\n'); // Read the data as a string
    Serial.print("Received data: ");
    Serial.println(data);

    // Process the received data
    processData(data);
  }
}

// Attach the servo motors to their pins and set initial positions
void initializeServos() {
  for (int i = 0; i < numServos; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(75);           // Set each servo to an initial position
  }
  delay(500);                      // Wait to ensure all servos move to their initial position
}

// Process the received data and control the servos or LED
void processData(String data) {
  data.trim();
  if (data.startsWith("[") && data.endsWith("]")) {
    data = data.substring(1, data.length() - 1);
  }

  int startIndex = 0;
  int separatorIndex = data.indexOf("][");

  while (separatorIndex != -1) {
    String segment = data.substring(startIndex, separatorIndex);
    handleCommand(segment);

    startIndex = separatorIndex + 2;
    separatorIndex = data.indexOf("][", startIndex);
  }

  if (startIndex < data.length()) {
    String lastSegment = data.substring(startIndex);
    handleCommand(lastSegment);
  }
}

// Handles individual commands for servo and LED
void handleCommand(String command) {
  command.trim();
  command.replace("\"", ""); // Remove quotes

  int separatorIndex = command.indexOf(',');

  if (separatorIndex != -1) {
    String commandType = command.substring(0, separatorIndex); // e.g., "LED" or "1"
    String value = command.substring(separatorIndex + 1); // e.g., "1" or "0"

    if (commandType == "LED") {
      int ledValue = value.toInt(); // Convert string to integer
      if (ledValue == 1) {
        digitalWrite(ledPin, HIGH);
        Serial.println("LED ON");
      } else if (ledValue == 0) {
        digitalWrite(ledPin, LOW);
        Serial.println("LED OFF");
      } else {
        Serial.println("Invalid LED Command Value: " + value);
      }
    } else {
      int servoIndex = commandType.toInt();
      int angle = value.toInt();

      if (servoIndex >= 1 && servoIndex <= numServos && angle >= 0 && angle <= 180) {
        setServoAngle(servoIndex, angle);
      } else {
        Serial.println("Invalid Servo Index or Angle: " + command);
      }
    }
  } else {
    Serial.println("Invalid Command Format: " + command);
  }
}

// Set the servo to a specific angle
void setServoAngle(int servoIndex, int angle) {
  if (servoIndex >= 1 && servoIndex <= numServos) {
    servos[servoIndex - 1].write(angle); // Servo index is 1-based, array index is 0-based
    Serial.println("Servo " + String(servoIndex) + " Angle: " + String(angle));
  } else {
    Serial.println("Invalid Servo Index: " + String(servoIndex));
  }
}
