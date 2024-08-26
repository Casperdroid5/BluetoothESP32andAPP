#include <ESP32Servo.h>
#include "BluetoothSerial.h"

// Define constants
const int numServos = 6;
const int ledPin = 2;
const int initialServoPosition = 75;

// BluetoothSerial object
BluetoothSerial SerialBT;

// Class to manage the servo motors
class ServoController {
  public:
    ServoController(const int* pins, int count);
    void initialize();
    void setAngle(int servoIndex, int angle);

  private:
    Servo* servos;
    int numServos;
    const int* servoPins;
};

ServoController::ServoController(const int* pins, int count) : servoPins(pins), numServos(count) {
  servos = new Servo[numServos];
}

void ServoController::initialize() {
  for (int i = 0; i < numServos; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(initialServoPosition);
  }
  delay(500);
}

void ServoController::setAngle(int servoIndex, int angle) {
  if (servoIndex >= 1 && servoIndex <= numServos) {
    servos[servoIndex - 1].write(angle);  // Servo index is 1-based, array index is 0-based
    Serial.println("Servo " + String(servoIndex) + " Angle: " + String(angle));
  } else {
    Serial.println("Invalid Servo Index: " + String(servoIndex));
  }
}

// Class to manage the LED
class LEDController {
  public:
    LEDController(int pin);
    void initialize();
    void setState(int state);

  private:
    int ledPin;
};

LEDController::LEDController(int pin) : ledPin(pin) {}

void LEDController::initialize() {
  pinMode(ledPin, OUTPUT);
}

void LEDController::setState(int state) {
  digitalWrite(ledPin, state == 1 ? HIGH : LOW);
  Serial.println(state == 1 ? "LED ON" : "LED OFF");
}

// Data processing class
class CommandProcessor {
  public:
    CommandProcessor(ServoController& servos, LEDController& led);
    void processData(String data);

  private:
    ServoController& servoController;
    LEDController& ledController;
    void handleCommand(String command);
};

CommandProcessor::CommandProcessor(ServoController& servos, LEDController& led)
  : servoController(servos), ledController(led) {}

void CommandProcessor::processData(String data) {
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

void CommandProcessor::handleCommand(String command) {
  command.trim();
  command.replace("\"", "");

  int separatorIndex = command.indexOf(',');

  if (separatorIndex != -1) {
    String commandType = command.substring(0, separatorIndex);
    String value = command.substring(separatorIndex + 1);

    if (commandType == "LED") {
      int ledValue = value.toInt();
      if (ledValue == 0 || ledValue == 1) {
        ledController.setState(ledValue);
      } else {
        Serial.println("Invalid LED Command Value: " + value);
      }
    } else {
      int servoIndex = commandType.toInt();
      int angle = value.toInt();

      if (servoIndex >= 1 && servoIndex <= numServos && angle >= 0 && angle <= 180) {
        servoController.setAngle(servoIndex, angle);
      } else {
        Serial.println("Invalid Servo Index or Angle: " + command);
      }
    }
  } else {
    Serial.println("Invalid Command Format: " + command);
  }
}

// Create instances of the controllers
const int servoPins[numServos] = {12, 10, 25, 26, 33, 32};
ServoController servoController(servoPins, numServos);
LEDController ledController(ledPin);
CommandProcessor commandProcessor(servoController, ledController);

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_BT_6DOF-Arm");

  // Initialize servo motors and LED
  servoController.initialize();
  ledController.initialize();

  Serial.println("ESP32 Ready");
}

void loop() {
  if (SerialBT.available()) {
    String data = SerialBT.readStringUntil('\n');
    Serial.print("Received data: ");
    Serial.println(data);

    // Process the received data
    commandProcessor.processData(data);
  }
}
