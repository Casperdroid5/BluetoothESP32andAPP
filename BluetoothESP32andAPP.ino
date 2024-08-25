#include <ESP32Servo.h>
#include "BluetoothSerial.h"

// Aantal servo motoren
const int numServos = 6;

// Servo-objecten en pinconfiguraties
Servo servos[numServos];
const int servoPins[numServos] = {12, 10, 25, 26, 33, 32};

// BluetoothSerial object maken
BluetoothSerial SerialBT;

// LED pin
const int ledPin = 2;

void setup() {
  Serial.begin(115200);            // Start USB serial communicatie voor debugging
  SerialBT.begin("ESP32_BT");      // Start Bluetooth met de naam ESP32_BT

  // Koppel servo motoren aan de pinnen en stel een beginpositie in
  initializeServos();
  
  pinMode(ledPin, OUTPUT);         // Stel de LED pin in als output
  Serial.println("ESP32 Bluetooth Multiple Servo & LED Control Ready.");
}

void loop() {
  if (SerialBT.available()) {      // Controleer of er data beschikbaar is via Bluetooth
    String data = SerialBT.readStringUntil('\n'); // Lees de data als een string
    Serial.print("Received data: ");
    Serial.println(data);

    // Verwerk de ontvangen data
    processData(data);
  }
}

// Koppel de servo motoren aan de pinnen en stel een beginpositie in
void initializeServos() {
  for (int i = 0; i < numServos; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(75);           // Zet elke servo op een beginpositie
  }
  delay(500);                      // Wacht om ervoor te zorgen dat alle servo's naar hun beginpositie bewegen
}

// Verwerkt de ontvangen data en stuurt de servo's aan
void processData(String data) {
  // Verwijder vierkante haken en spaties
  data.trim();
  if (data.startsWith("[") && data.endsWith("]")) {
    data = data.substring(1, data.length() - 1);
  }

  // Verwerk elk segment van de ontvangen data
  int startIndex = 0;
  int separatorIndex = data.indexOf("][");

  while (separatorIndex != -1) {
    String segment = data.substring(startIndex, separatorIndex);
    handleServoCommand(segment);

    startIndex = separatorIndex + 2;
    separatorIndex = data.indexOf("][", startIndex);
  }

  // Verwerk het laatste segment
  if (startIndex < data.length()) {
    String lastSegment = data.substring(startIndex);
    handleServoCommand(lastSegment);
  }
}

// Verwerkt individuele servo-opdrachten
void handleServoCommand(String command) {
  command.trim();
  command.replace("\"", "");

  int separatorIndex = command.indexOf(',');

  if (separatorIndex != -1) {
    int servoIndex = command.substring(0, separatorIndex).toInt(); // Lees de servo index
    int angle = command.substring(separatorIndex + 1).toInt(); // Lees de hoek

    // Serial.print("Servo Index: ");
    // Serial.println(servoIndex);
    // Serial.print("Angle: ");
    // Serial.println(angle);

    // Verplaats de juiste servo naar de opgegeven hoek
    if (servoIndex >= 1 && servoIndex <= numServos && angle >= 0 && angle <= 180) {
      setServoAngle(servoIndex, angle);
    } else {
      Serial.println("Invalid Servo Index or Angle");
    }
  } else {
    Serial.println("Invalid Command Format");
  }
}

// Zet de servo op een specifieke hoek
void setServoAngle(int servoIndex, int angle) {
  if (servoIndex >= 1 && servoIndex <= numServos) {
    servos[servoIndex - 1].write(angle); // Servo index is 1-based, array index is 0-based
    Serial.println("Servo " + String(servoIndex) + " Angle: " + String(angle));
  } else {
    Serial.println("Invalid Servo Index");
  }
}
