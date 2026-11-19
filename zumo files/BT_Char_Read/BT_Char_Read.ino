
#include <Wire.h>
#include <ZumoShield.h>
#include <SoftwareSerial.h>

// Existing working Bluetooth pins
SoftwareSerial bluetooth(2, 4);

ZumoMotors motors;

// Motor settings
const int NORMAL_SPEED = 300;
const int NORMAL_TURN_SPEED = 180;

const int FAST_SPEED = 400;
const int FAST_TURN_SPEED = 300;

// Bluetooth failsafe
const unsigned long COMMAND_TIMEOUT = 400;

unsigned long lastCommandTime = 0;

char currentCommand = 'S';
bool fastMode = false;

// Apply motor movement
void driveRobot()
{
    int speed;
    int turnSpeed;

    if (fastMode)
    {
        speed = FAST_SPEED;
        turnSpeed = FAST_SPEED;
    }
    else
    {
        speed = NORMAL_SPEED;
        turnSpeed = NORMAL_TURN_SPEED;
    }

    switch (currentCommand)
    {
        case 'F':
            motors.setSpeeds(speed, speed);
            break;

        case 'B':
            motors.setSpeeds(-speed, -speed);
            break;

        case 'L':
            motors.setSpeeds(-turnSpeed, turnSpeed);
            break;

        case 'R':
            motors.setSpeeds(turnSpeed, -turnSpeed);
            break;

        case 'S':
        default:
            motors.setSpeeds(0, 0);
            break;
    }
}

void setup()
{
    Serial.begin(9600);
    bluetooth.begin(9600);

    // Start safely
    currentCommand = 'S';
    fastMode = false;

    motors.setSpeeds(0, 0);

    lastCommandTime = millis();

    Serial.println("ZumoBot Ready");
    Serial.println("Normal Mode Active");
}

void loop()
{
    while (bluetooth.available() > 0)
    {
        char received = bluetooth.read();

        // Fast mode ON
        if (received == 'T')
        {
            fastMode = true;
            lastCommandTime = millis();

            Serial.println("FAST MODE ON");
            driveRobot();
        }

        // Fast mode OFF
        else if (received == 'N')
        {
            fastMode = false;
            lastCommandTime = millis();

            Serial.println("NORMAL MODE ON");
            driveRobot();
        }

        // Joystick movement
        else if (received == 'F' ||
                 received == 'B' ||
                 received == 'L' ||
                 received == 'R' ||
                 received == 'S')
        {
            lastCommandTime = millis();

            if (received != currentCommand)
            {
                currentCommand = received;

                Serial.print("Command: ");
                Serial.println(currentCommand);

                driveRobot();
            }
        }
    }

    // Stop if Bluetooth messages disappear
    if (millis() - lastCommandTime >
        COMMAND_TIMEOUT)
    {
        if (currentCommand != 'S')
        {
            currentCommand = 'S';
            driveRobot();

            Serial.println("Bluetooth Lost - STOP");
        }
    }
}
