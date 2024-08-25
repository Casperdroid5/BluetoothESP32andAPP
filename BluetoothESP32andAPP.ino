#include <ESP32Servo.h>
#include "BluetoothSerial.h"

Servo myServo;                    // Maak een Servo object
BluetoothSerial SerialBT;          // Maak een BluetoothSerial object

const int ledPin = 2;              // LED verbonden met GPIO2 (pas aan naar je eigen setup)

void setup() {
  Serial.begin(115200);            // Start USB serial communicatie voor debugging
  SerialBT.begin("ESP32_BT");      // Start Bluetooth met de naam ESP32_BT
  myServo.attach(32);               // Verbind de servo met pin 9 (pas aan naar je eigen setup)
  myServo.write(90);               // Zet de servo op de middelste positie
  pinMode(ledPin, OUTPUT);         // Stel de ledPin in als output
  Serial.println("ESP32 Bluetooth Servo & LED Control Ready.");
}

void loop() {
  if (SerialBT.available()) {      // Controleer of er data beschikbaar is via Bluetooth
    String data = SerialBT.readStringUntil('\n'); // Lees de data als een string

    // Controleer of de ontvangen data een "angle" of LED commando bevat
    if (data == "1") {
      digitalWrite(ledPin, HIGH);  // Zet de LED aan als "1" ontvangen wordt
      Serial.println("LED ON");
    } else if (data == "0") {
      digitalWrite(ledPin, LOW);   // Zet de LED uit als "0" ontvangen wordt
      Serial.println("LED OFF");
    } else {
      int angle = data.toInt();    // Converteer de ontvangen data naar een integer waarde voor de servo

      // Controleer of de hoek binnen het bereik van 0 tot 180 graden ligt
      if (angle >= 0 && angle <= 180) {
        myServo.write(angle);      // Zet de servo in de juiste positie
        Serial.println("Angle: " + String(angle)); // Print de hoek naar de Serial Monitor
      }
    }
  }
}
