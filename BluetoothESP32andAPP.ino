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
int speedDelay = 0;       // Default delay for servo speed
String dataIn = "";

// Function declarations
void initializeServos();
void processData(String data);
void handleCommand(String command);
void moveServo(int servoIndex, float angle);
void savePositions();
void runSavedPositions();
void resetPositions();
int mapSpeedToDelay(int speed);
bool pauseRunning = false;

void setup() {
    Serial.begin(115200);        // Start USB serial communication for debugging
    SerialBT.begin("ESP32_BT_6DOF-Arm");  // Start Bluetooth with the name ESP32_BT

    // Wait for a Bluetooth connection
    Serial.println("Arm Ready, waiting for device to pair...");
    while (!SerialBT.hasClient()) {
        delay(50);  // Check every 10ms
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
    delay(250);  // Wait to ensure all servos move to their initial position
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
            pauseRunning = false;
        } else if (command.equalsIgnoreCase("PAUSE")) {
            //pauseRunning = !pauseRunning; // Toggle the pauseRunning state
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

    int currentIndex = 0; // Track the current step index

    while (true) { // Infinite loop
        for (int i = currentIndex; i < servoIndex; i++) {
            for (int j = 0; j < numServos; j++) {
                // Check for PAUSE or RESET command before moving each servo
                if (SerialBT.available()) {
                    String incomingCommand = SerialBT.readString();
                    incomingCommand.trim();
                    Serial.println("Received command: " + incomingCommand);

                    if (incomingCommand.equalsIgnoreCase("(PAUSE)")) {
                        pauseRunning = true;
                        Serial.println("Execution paused.");
                    } else if (incomingCommand.equalsIgnoreCase("(RESET)")) {
                        resetPositions();
                        Serial.println("Execution reset.");
                        return;  // Exit the function to stop further execution
                    }
                }

                // If pause is triggered, stop execution
                while (pauseRunning) {
                    if (SerialBT.available()) {
                        String incomingCommand = SerialBT.readString();
                        incomingCommand.trim();
                        Serial.println("Received during pause: " + incomingCommand);

                        if (incomingCommand.equalsIgnoreCase("(RUN)")) {
                            pauseRunning = false;
                            Serial.println("Resuming execution.");
                        } else if (incomingCommand.equalsIgnoreCase("(RESET)")) {
                            resetPositions();
                            Serial.println("Execution reset.");
                            return;  // Exit the function to stop further execution
                        }
                    }
                    delay(100);  // Small delay to prevent tight loop
                }

                // Move the servo if not paused
                if (!pauseRunning) {
                    Serial.print("Running Servo ");
                    Serial.print(j + 1);
                    Serial.print(" to Position: ");
                    Serial.println(servoSP[j][i]);
                    servos[j].write(servoSP[j][i]);
                    delay(speedDelay);
                }
            }

            // Check again after all servos have been moved in the step
            if (pauseRunning || !SerialBT.hasClient()) {
                currentIndex = i;  // Save current position before pausing
                break;  // Exit the loop if paused or Bluetooth connection lost
            }

            delay(1000);  // Optional: Add delay between steps to observe movement
        }

        // Reset the currentIndex to 0 to repeat the sequence indefinitely
        currentIndex = 0;

        // Check for reset after completing one cycle
        if (pauseRunning || !SerialBT.hasClient()) {
            break;  // Exit the loop if paused or Bluetooth connection lost
        }
    }
}




void resetPositions() {
    for (int i = 0; i < numServos; i++) {
        memset(servoSP[i], 0, sizeof(servoSP[i]));
    }
    servoIndex = 0;
    Serial.println("Positions Reset");
}


int mapSpeedToDelay(int speed) {
    // Inverse mapping: 220 (speed) -> 0 (delay), 0 (speed) -> 20 (delay), maximum of 20 delay
    return 20 - speed;
}
