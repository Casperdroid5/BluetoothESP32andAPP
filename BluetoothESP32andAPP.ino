#include <ESP32Servo.h>
#include "BluetoothSerial.h"

// Number of servo motors
const int numServos = 6;
const int ledPin = 2;

Servo servos[numServos];
const int servoPins[numServos] = {12, 10, 25, 26, 33, 32};

BluetoothSerial SerialBT;

// Data storage and control
int servoPos[numServos];
int servoPPos[numServos];
int servoSP[numServos][50]; // For storing positions/steps
int servoIndex = 0;        // Index for saving positions
int speedDelay = 0;       // Delay for servo speed
String dataIn = "";

// Function declarations
void initializeServos();
void processData(String data);
void handleCommand(String command);
void moveServo(int servoIndex, float angle);
void savePositions();
void runSavedPositions();
void resetPositions();
void pauseRunning();

void setup() {
    Serial.begin(115200);        // Start USB serial communication for debugging
    SerialBT.begin("ESP32_BT_6DOF-Arm");  // Start Bluetooth with the name ESP32_BT

    // Attach servos to their pins and set initial positions
    initializeServos();
    
    pinMode(ledPin, OUTPUT);  // Set LED pin as output
    Serial.println("ESP32 Ready");
}

void loop() {
    if (SerialBT.available()) {  // Check if data is available via Bluetooth
        dataIn = SerialBT.readString();  // Read the data as a string
        Serial.print("Received data: ");
        Serial.println(dataIn);

        // Process the received data
        processData(dataIn);
    }
}

void initializeServos() {
    for (int i = 0; i < numServos; i++) {
        servos[i].attach(servoPins[i]);
        servoPPos[i] = 90;  // Set initial position to 90 degrees
        servos[i].write(servoPPos[i]);
    }
    delay(500);  // Wait to ensure all servos move to their initial position
}

void processData(String data) {
    data.trim();
    
    int startIndex = data.indexOf('(');
    int endIndex = data.indexOf(')');
    
    while (startIndex != -1 && endIndex != -1) {
        String command = data.substring(startIndex + 1, endIndex);  // Extract command within parentheses
        handleCommand(command);
        
        startIndex = data.indexOf('(', endIndex);
        endIndex = data.indexOf(')', startIndex);
    }
}

void handleCommand(String command) {
    command.trim();
    
    Serial.println("Handling Command: " + command);
    
    int separatorIndex = command.indexOf(' ');
    
    if (separatorIndex != -1) {
        String action = command.substring(0, separatorIndex);
        String parameters = command.substring(separatorIndex + 1);
        
        if (action.startsWith("s")) {
            // Servo movement command
            int servoIndex = action.substring(1).toInt() - 1;  // Convert to 0-based index
            float angle = parameters.toFloat();
            
            if (servoIndex >= 0 && servoIndex < numServos && angle >= 0 && angle <= 180) {
                moveServo(servoIndex, angle);
            } else {
                Serial.println("Invalid Servo Index or Angle: " + command);
            }
        } else if (action.length() == 1 && isDigit(action[0])) {
            // Handle commands without the 's' prefix
            int servoIndex = action.toInt() - 1;  // Convert to 0-based index
            float angle = parameters.toFloat();
            
            if (servoIndex >= 0 && servoIndex < numServos && angle >= 0 && angle <= 180) {
                moveServo(servoIndex, angle);
            } else {
                Serial.println("Invalid Servo Index or Angle: " + command);
            }
        } else if (action.startsWith("d")) {
            // Speed adjustment
            speedDelay = parameters.toInt();
            Serial.println("Speed set to: " + String(speedDelay));
        } else if (action == "SAVE") {
            savePositions();
        } else if (action == "RUN") {
            runSavedPositions();
        } else if (action == "RESET") {
            resetPositions();
        } else if (action == "PAUSE") {
            pauseRunning();
        } else {
            Serial.println("Invalid Command: " + command);
        }
    } else {
        Serial.println("Invalid Command Format: " + command);
    }
}

void moveServo(int servoIndex, float angle) {
    if (servoIndex >= 0 && servoIndex < numServos) {
        if (servoPPos[servoIndex] > angle) {
            for (float j = servoPPos[servoIndex]; j >= angle; j--) {
                servos[servoIndex].write(j);
                delay(speedDelay);
            }
        } else if (servoPPos[servoIndex] < angle) {
            for (float j = servoPPos[servoIndex]; j <= angle; j++) {
                servos[servoIndex].write(j);
                delay(speedDelay);
            }
        }
        servoPPos[servoIndex] = angle;
        Serial.println("Servo " + String(servoIndex + 1) + " Angle: " + String(angle));
    } else {
        Serial.println("Invalid Servo Index: " + String(servoIndex + 1));
    }
}

void savePositions() {
    for (int i = 0; i < numServos; i++) {
        servoSP[i][servoIndex] = servoPPos[i];
    }
    servoIndex++;
    Serial.println("Positions Saved. Index: " + String(servoIndex));
}

void runSavedPositions() {
    while (dataIn != "RESET") {
        for (int i = 0; i < servoIndex - 1; i++) {
            for (int j = 0; j < numServos; j++) {
                if (servoSP[j][i] > servoSP[j][i + 1]) {
                    for (int k = servoSP[j][i]; k >= servoSP[j][i + 1]; k--) {
                        servos[j].write(k);
                        delay(speedDelay);
                    }
                } else if (servoSP[j][i] < servoSP[j][i + 1]) {
                    for (int k = servoSP[j][i]; k <= servoSP[j][i + 1]; k++) {
                        servos[j].write(k);
                        delay(speedDelay);
                    }
                }
            }
        }
    }
    Serial.println("Running Saved Positions Stopped");
}

void resetPositions() {
    for (int i = 0; i < numServos; i++) {
        memset(servoSP[i], 0, sizeof(servoSP[i]));
    }
    servoIndex = 0;
    Serial.println("Positions Reset");
}

void pauseRunning() {
    while (dataIn != "RUN") {
        if (SerialBT.available()) {
            dataIn = SerialBT.readStringUntil('\n');
            if (dataIn == "RESET") {
                resetPositions();
                break;
            }
        }
    }
    Serial.println("Paused. Waiting for RUN command.");
}
