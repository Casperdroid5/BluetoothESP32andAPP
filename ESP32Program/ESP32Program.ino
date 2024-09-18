#include <ESP32Servo.h>
#include "BluetoothSerial.h"

const int maxSteps = 50;  // Maximum number of steps that can be saved with app
const int BluetoothIndicator = 2; // LED for bluetooth connection status


// Global hysteresis value (in degrees)
const float hysteresis = 8.0;  // Hysteresis value in degrees
const int numServos = 6;
Servo servos[numServos];
const int servoPins[numServos] = {13, 12, 14, 18, 19, 5}; // waste = s1, shoulder = s2, elbow = s3, wrist roll = s4, wrist pitch =s5, grip = s6
bool initservos = false;

BluetoothSerial SerialBT;

// Data storage and control
int servoPos[numServos];
int servoPPos[numServos];
int servoSP[numServos][maxSteps]; // For storing positions/steps
int servoIndex = 0;        // Index for saving positions
int speedDelay = 0;       // Default delay for servo speed
String dataIn = "";

bool pauseRunning = false;
bool runningPositions = false;  // Track if we are currently running saved positions
int currentStepIndex = 0;       // Track the current step in runSavedPositions



// Function declarations
void initializeServos();
void processData(String data);
void handleCommand(String command);
void moveServo(int servoIndex, float angle);
void savePositions();
void runSavedPositions();
void resetPositions();
void checkForCommands();
int mapSpeedToDelay(int speed);

void setup() {
    Serial.begin(115200);        // Start USB serial communication for debugging
    delay(100);
    SerialBT.begin("6DOF-Robot-Arm");  // Start Bluetooth with the name ESP32_BT

    Serial.println("Arm Ready, waiting for device to pair...");
    while (!SerialBT.hasClient()) {
        delay(50);  // Check every 50ms
    }
    Serial.println("Device paired successfully!");
    pinMode(BluetoothIndicator, OUTPUT);  // Set LED pin as output
    digitalWrite(BluetoothIndicator, HIGH);  // Turn on LED when reconnected
    int initialSpeed = 15; // initial speed
    speedDelay = mapSpeedToDelay(initialSpeed); // set speed

    // Attach servos to their pins and set initial positions
    initializeServos();
}

void loop() {
    checkForCommands();  // Continuously check for incoming commands

    // Check if Bluetooth connection is lost
    if (!SerialBT.hasClient()) {
        digitalWrite(BluetoothIndicator, LOW);  // Turn off LED when disconnected
        initservos = false;
        int initialSpeed = 15;  // reset speed after disconnect
        speedDelay = mapSpeedToDelay(initialSpeed); // set speed
        initializeServos();
        Serial.println("Device disconnected!");
        while (!SerialBT.hasClient()) {
            delay(50);  // Wait until a device connects again
        }
        Serial.println("Device reconnected!");
        digitalWrite(BluetoothIndicator, HIGH);  // Turn on LED when reconnected
    }
    if (initservos == false) {
        initializeServos();
        initservos = true;

        // Set default speed to 50% at startup
        int initialSpeed = 15;  // Corresponds to 75% speed in mapSpeedToDelay()
        speedDelay = mapSpeedToDelay(initialSpeed);  // Set the initial speed delay
    }

    if (runningPositions && !pauseRunning) {
        runSavedPositions();
    }
}

void initializeServos() {
    // Define the initial positions for each servo
    int initialPositions[numServos] = {90, 45, 0, 85, 105, 35};  // Adjust these values to your preferred initial positions

    for (int i = 0; i < numServos; i++) {
        servos[i].attach(servoPins[i]);
        servoPPos[i] = initialPositions[i];  // Set each servo to its corresponding initial position
        servos[i].write(servoPPos[i]);       // Move the servo to the initial position
    }
    delay(250);  // Wait to ensure all servos move to their initial positions
}

void checkForCommands() {
    if (SerialBT.available()) {
        dataIn = SerialBT.readString();
        Serial.print("Received data: ");
        Serial.println(dataIn);
        processData(dataIn);
    }
}

void processData(String data) {
    data.trim();
    int startIndex = 0;

    while (startIndex != -1) {
        int endIndex = data.indexOf(')', startIndex);
        if (endIndex != -1) {
            String command = data.substring(startIndex + 1, endIndex);
            handleCommand(command);
            startIndex = data.indexOf('(', endIndex);
        } else {
            startIndex = -1;
        }
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
            int servoIndex = action.substring(1).toInt() - 1;
            float angle = parameters.toFloat();

            if (servoIndex >= 0 && servoIndex < numServos && angle >= 0 && angle <= 180) {
                moveServo(servoIndex, angle);
            } else {
                Serial.println("Invalid Servo Index or Angle: " + command);
            }
        } else if (action.startsWith("d")) {
            int speed = parameters.toInt();
            speedDelay = mapSpeedToDelay(speed);
            Serial.println("Speed set to: " + String(speed) + " (Delay: " + String(speedDelay) + ")");
        } else {
            Serial.println("Invalid Command Format: " + command);
        }
    } else {
        if (command.equalsIgnoreCase("SAVE")) {
            savePositions();
        } else if (command.equalsIgnoreCase("RUN")) {
            pauseRunning = false;
            runningPositions = true;
            currentStepIndex = 0;  // Reset to the beginning of the sequence
        } else if (command.equalsIgnoreCase("RESET")) {
            resetPositions();
            pauseRunning = false;
            runningPositions = false;
        } else if (command.equalsIgnoreCase("PAUSE")) {
            pauseRunning = true;  // Toggle the pause state
        } else {
            Serial.println("Invalid Command: " + command);
        }
    }
}

void moveServo(int servoIndex, float angle) {
    if (servoIndex >= 0 && servoIndex < numServos) {
        int start = servoPPos[servoIndex];
        int end = angle;

        // Check if the change is greater than the hysteresis value
        if (abs(end - start) > hysteresis) {
            for (int pos = start; pos != end; pos += (start < end ? 1 : -1)) {
                servos[servoIndex].write(pos);
                delay(speedDelay);
                checkForCommands();  // Continuously check for commands during movement
            }
            servoPPos[servoIndex] = angle;
            Serial.println("Servo " + String(servoIndex + 1) + " Angle: " + String(angle));
        } else {
            Serial.println("Movement ignored due to hysteresis.");
        }
    } else {
        Serial.println("Invalid Servo Index: " + String(servoIndex + 1));
    }
}

void savePositions() {
    if (servoIndex < maxSteps) {
        for (int i = 0; i < numServos; i++) {
            servoSP[i][servoIndex] = servoPPos[i];
        }
        Serial.println("Saved Position at Index " + String(servoIndex));
        servoIndex++;
    } else {
        Serial.println("Error: Maximum number of saved positions reached.");
    }
}

void runSavedPositions() {
    Serial.println("Running saved positions...");

    for (int i = currentStepIndex; i < servoIndex; i++) {
        for (int j = 0; j < numServos; j++) {
            checkForCommands();  // Check for new commands during execution

            if (!pauseRunning) {
                // Use the moveServo function to move the servo to the saved position
                moveServo(j, servoSP[j][i]);
            } else {
                currentStepIndex = i;
                return;  // Pause execution and return to loop
            }
        }
        delay(1000);  // Optional: Delay between steps
    }

    currentStepIndex = 0;  // Reset to allow repeating the sequence
}

void resetPositions() {
    for (int i = 0; i < numServos; i++) {
        memset(servoSP[i], 0, sizeof(servoSP[i]));
    }
    servoIndex = 0;
    Serial.println("Positions Reset");
}

int mapSpeedToDelay(int speed) {
    return 20 - speed;
}
