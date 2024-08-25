#include <ESP32Servo.h>
#include "BluetoothSerial.h"

// Aantal servo motoren
const int numServos = 6; // We hebben nu 6 servo motoren

// Maak servo-objecten aan
Servo servos[numServos]; // Array om 6 servo-objecten op te slaan

BluetoothSerial SerialBT; // Maak een BluetoothSerial object

const int ledPin = 2; // LED verbonden met GPIO2 (pas aan naar je eigen setup)

// Pin configuratie voor de servo's (pas deze aan naar je eigen setup)
const int servoPins[numServos] = {32, 33, 25, 26, 27, 14}; // Servo pinnen, pas deze aan zoals nodig

void setup() {
  Serial.begin(115200); // Start USB serial communicatie voor debugging
  SerialBT.begin("ESP32_BT"); // Start Bluetooth met de naam ESP32_BT

  // Koppel servo motoren aan de pinnen
  for (int i = 0; i < numServos; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(75); // Zet elke servo op een tussenpositie
    delay(500); // Laat de servo tijd om naar de beginpositie te bewegen
  }

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
    if (servoIndex >= 1 && servoIndex <= numServos && angle >= 0 && angle <= 180) {
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
  if (servoIndex >= 1 && servoIndex <= numServos) {
    servos[servoIndex - 1].write(angle); // Servo index is 1-based, array index is 0-based
    Serial.println("Servo " + String(servoIndex) + " Angle: " + String(angle));
  }
}
