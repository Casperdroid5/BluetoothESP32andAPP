#include <ESP32Servo.h>
#include "BluetoothSerial.h"

// Maak aparte servo-objecten aan
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;
Servo servo6;

BluetoothSerial SerialBT; // Maak een BluetoothSerial object

const int ledPin = 2; // LED verbonden met GPIO2 (pas aan naar je eigen setup)

// Pin configuratie voor de servo's (pas deze aan naar je eigen setup)
const int servoPin1 = 12;
const int servoPin2 = 10;
const int servoPin3 = 25;
const int servoPin4 = 26;
const int servoPin5 = 33;
const int servoPin6 = 32;

void setup() {
  Serial.begin(115200); // Start USB serial communicatie voor debugging
  SerialBT.begin("ESP32_BT"); // Start Bluetooth met de naam ESP32_BT

  // Koppel servo motoren aan de pinnen
  servo1.attach(servoPin1);
  servo2.attach(servoPin2);
  servo3.attach(servoPin3);
  servo4.attach(servoPin4);
  servo5.attach(servoPin5);
  servo6.attach(servoPin6);

  // Zet elke servo op een beginpositie
  servo1.write(75);
  servo2.write(75);
  servo3.write(75);
  servo4.write(75);
  servo5.write(75);
  servo6.write(75);

  pinMode(ledPin, OUTPUT); // Stel de ledPin in als output
  Serial.println("ESP32 Bluetooth Multiple Servo & LED Control Ready.");
}

void loop() {
  if (SerialBT.available()) { // Controleer of er data beschikbaar is via Bluetooth
    String data = SerialBT.readStringUntil('\n'); // Lees de data als een string

    Serial.print("Received data: ");
    Serial.println(data);

    // Verwerk de ontvangen data
    processData(data);
  }
}

// Verwerk de ontvangen data en stuur de servo's aan
void processData(String data) {
  // Verwijder de vierkante haken en extra spaties
  data.trim();
  if (data.startsWith("[") && data.endsWith("]")) {
    data = data.substring(1, data.length() - 1);
  }

  int startIndex = 0;
  int separatorIndex = data.indexOf("][");

  while (separatorIndex != -1) {
    // Verkrijg elk commando
    String segment = data.substring(startIndex, separatorIndex);
    handleServoCommand(segment);

    // Verwijder het verwerkte segment en update de startIndex
    startIndex = separatorIndex + 2;
    separatorIndex = data.indexOf("][", startIndex);
  }

  // Verwerk het laatste segment (of enige resterende data)
  if (startIndex < data.length()) {
    String lastSegment = data.substring(startIndex);
    handleServoCommand(lastSegment);
  }
}

// Verwerkt individuele servo-opdrachten
void handleServoCommand(String command) {
  // Verwijder de extra aanhalingstekens en spaties
  command.trim();
  command.replace("\"", "");

  int separatorIndex = command.indexOf(',');

  if (separatorIndex != -1) {
    int servoIndex = command.substring(0, separatorIndex).toInt(); // Lees de servo index
    int angle = command.substring(separatorIndex + 1).toInt(); // Lees de hoek

    Serial.print("Servo Index: ");
    Serial.println(servoIndex);
    Serial.print("Angle: ");
    Serial.println(angle);

    // Controleer of de servo index en hoek binnen de verwachte waarden liggen
    if (servoIndex >= 1 && servoIndex <= 6 && angle >= 0 && angle <= 180) {
      // Verplaats de juiste servo naar de opgegeven hoek
      setServoAngle(servoIndex, angle); // Gebruik de functie om de servo-stand in te stellen
    } else {
      Serial.println("Invalid Servo Index or Angle");
    }
  } else {
    Serial.println("Invalid Command Format");
  }
}

// Functie om de servo op een specifieke hoek te zetten
void setServoAngle(int servoIndex, int angle) {
  switch (servoIndex) {
    case 1:
      servo1.write(angle);
      Serial.println("Servo 1 Angle: " + String(angle));
      break;
    case 2:
      servo2.write(angle);
      Serial.println("Servo 2 Angle: " + String(angle));
      break;
    case 3:
      servo3.write(angle);
      Serial.println("Servo 3 Angle: " + String(angle));
      break;
    case 4:
      servo4.write(angle);
      Serial.println("Servo 4 Angle: " + String(angle));
      break;
    case 5:
      servo5.write(angle);
      Serial.println("Servo 5 Angle: " + String(angle));
      break;
    case 6:
      servo6.write(angle);
      Serial.println("Servo 6 Angle: " + String(angle));
      break;
    default:
      Serial.println("Invalid Servo Index");
      break;
  }
}
