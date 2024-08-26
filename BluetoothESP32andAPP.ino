#include <ESP32Servo.h>
#include "BluetoothSerial.h"

// Number of servo motors
const int numServos = 6;
const int maxSteps = 50;  // Maximum number of steps that can be saved
const int ledPin = 2;

Servo servos[numServos];
const int servoPins[numServos] = {12, 10, 25, 26, 33, 32};

BluetoothSerial SerialBT;

// Data storage and control
int servoPos[numServos];
int servoPPos[numServos];
int servoSP[numServos][maxSteps]; // For storing positions/steps
int servoIndex = 0;        // Index for saving positions
int speedDelay = 15;       // Default delay for servo speed
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
int mapSpeedToDelay(int speed);

void setup() {
    Serial.begin(115200);        // Start USB serial communication for debugging
    SerialBT.begin("ESP32_BT_6DOF-Arm");  // Start Bluetooth with the name ESP32_BT

    // Wait for a Bluetooth connection
    Serial.println("Arm Ready, waiting for device to pair...");
    while (!SerialBT.hasClient()) {
        delay(100);  // Check every 100ms
    }
    Serial.println("Device paired successfully!");
    pinMode(ledPin, OUTPUT);  // Set LED pin as output


    // Attach servos to their pins and set initial positions
    initializeServos();
    

}

void loop() {
    if (SerialBT.available()) {  // Check if data is available via Bluetooth
        digitalWrite(ledPin, HIGH);
        dataIn = SerialBT.readString();  // Read the data as a string
        Serial.print("Received data: ");
        Serial.println(dataIn);

        // Process the received data
        processData(dataIn);
    } else {  // If no data is available
        digitalWrite(ledPin, LOW);
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
    
    int startIndex = 0;
    while (startIndex != -1) {
        int endIndex = data.indexOf(')', startIndex); // Find the closing parenthesis
        if (endIndex != -1) {
            String command = data.substring(startIndex + 1, endIndex); // Extract command within parentheses
            handleCommand(command);
            startIndex = data.indexOf('(', endIndex); // Move to the next command
        } else {
            startIndex = -1; // No more commands
        }
    }
}

void handleCommand(String command) {
    command.trim();
    
    Serial.println("Handling Command: " + command);
    
    int separatorIndex = command.indexOf(' ');
    
    if (separatorIndex != -1) {
        // Command with parameters, e.g., s6 54.6
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
        } else if (action.startsWith("d")) {
            // Speed adjustment
            int speed = parameters.toInt();
            speedDelay = mapSpeedToDelay(speed);
            Serial.println("Speed set to: " + String(speed) + " (Delay: " + String(speedDelay) + ")");
        } else {
            Serial.println("Invalid Command Format: " + command);
        }
    } else {
        // Simple command without parameters, e.g., SAVE or RUN
        if (command.equalsIgnoreCase("SAVE")) {
            savePositions();
        } else if (command.equalsIgnoreCase("RUN")) {
            runSavedPositions();
        } else if (command.equalsIgnoreCase("RESET")) {
            resetPositions();
        } else if (command.equalsIgnoreCase("PAUSE")) {
            pauseRunning();
        } else {
            Serial.println("Invalid Command: " + command);
        }
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
    if (servoIndex < maxSteps) {  // Prevent exceeding array bounds
        for (int i = 0; i < numServos; i++) {
            servoSP[i][servoIndex] = servoPPos[i];
        }
        Serial.print("Saved Position at Index ");
        Serial.println(servoIndex);
        for (int i = 0; i < numServos; i++) {
            Serial.print("Servo ");
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.println(servoSP[i][servoIndex]);
        }
        servoIndex++;
    } else {
        Serial.println("Error: Maximum number of saved positions reached.");
    }
}

void runSavedPositions() {
    Serial.println("Running saved positions...");
    for (int i = 0; i < servoIndex; i++) {
        Serial.print("Step ");
        Serial.println(i + 1);
        for (int j = 0; j < numServos; j++) {
            Serial.print("Running Servo ");
            Serial.print(j + 1);
            Serial.print(" to Position: ");
            Serial.println(servoSP[j][i]);
            servos[j].write(servoSP[j][i]);
        }
        delay(1000);  // Optional: Add delay between steps to observe movement
    }
    Serial.println("Completed running saved positions.");
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
            dataIn = SerialBT.readString();
            if (dataIn.equalsIgnoreCase("RESET")) {
                resetPositions();
                break;
            }
        }
    }
    Serial.println("Paused. Waiting for RUN command.");
}

int mapSpeedToDelay(int speed) {
    // Inverse mapping: 255 (speed) -> 0 (delay), 0 (speed) -> 255 (delay)
    return 255 - speed;
}
