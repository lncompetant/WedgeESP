#include <ESP32Servo.h>
#include <Bluepad32.h>

Servo escLeft;    // Left wheel motor
Servo escRight;   // Right wheel motor

const int driftoffset = 30;  //amount to offset joystick drift by
bool controllerConnected = false;
const float sensitivityPercentage = 0.50;  //permanent sensitivity modifier
float speedSensitivity = 0.5; //temp value to change speed mid fight
float turnSensitivity = 1; //temp value to change turn sensitivity mid fight
int inversion = 1;  //if the bot is flipped

// Motor control pins
const int leftPin = D9;
const int rightPin = D10;

// Controller pointer
ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// ESC initialization flag
bool escArmed = false;

// Function prototypes

void processJoysticks(ControllerPtr ctl);
void armESC();


const int ledPin = D0;  // the number of the power LED pin

// constants won't change:
const long interval = 1000;  // interval at which to blink (milliseconds)
void setup() {
  Serial.begin(115200);

  pinMode(2, OUTPUT); //controller connected LED on board
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin,HIGH);

  escLeft.attach(leftPin, 1000, 2000);    // Attach the ESC for the left wheel
  escRight.attach(rightPin, 1000, 2000);  // Attach the ESC for the right wheel

  // Initialize Bluepad32
  BP32.setup(&onConnectedController, &onDisconnectedController);

}

void loop() {
  digitalWrite(ledPin,HIGH);
  bool dataUpdated = BP32.update();


  // Process controller input
  if (dataUpdated) {
    for (auto ctl : myControllers) {
      if (ctl && ctl->isConnected()) {
        if (!escArmed) {
          armESC();
          escArmed = true;
        }
        processJoysticks(ctl);
      }
    }
  } else {
    //onDisconnectedController();
  }

  delay(20);
}

void onConnectedController(ControllerPtr ctl) {
  controllerConnected = true;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      myControllers[i] = ctl;
      break;
    }
  }
  Serial.println("Controller connected");
}

void onDisconnectedController(ControllerPtr ctl) {
  stop();
  controllerConnected = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      myControllers[i] = nullptr;
      break;
    }
  }
  Serial.println("Controller disconnected");
}


void processJoysticks(ControllerPtr ctl) {
  // Control the wheels using the joystick
  int leftyAxis = ctl->axisY();    // Assuming this is the Y-axis for forward/backward
  int rightxAxis = ctl->axisRX();  // Assuming this is the X-axis for left/right
  int rightyAxis = ctl->axisRY();
  //int RawRightYaxis = ctl->axisRY();
  int processedRight;
  int processedLeft;
  int mappedRight;
  int mappedLeft;
 
// the sign in front inverts the whole control, the sign after the leftyAxis inverts the turn
  processedLeft = inversion*(leftyAxis*speedSensitivity - (rightxAxis * sensitivityPercentage*turnSensitivity));
  processedRight = -inversion*(leftyAxis*speedSensitivity + (rightxAxis * sensitivityPercentage*turnSensitivity));

  if(abs(processedLeft) < driftoffset){
    mappedLeft = 0;
  }
  if(abs(processedRight)<driftoffset){
    mappedRight = 0;
  }
  
  Serial.print("Right: ");
  Serial.println(processedRight);
  Serial.print("Left: ");
  Serial.println(processedLeft);
  mappedLeft = map(processedLeft, -512, 512, 1000, 2000);
  mappedRight = map(processedRight, -512, 512, 1000, 2000);

  escLeft.writeMicroseconds(mappedLeft);
  escRight.writeMicroseconds(mappedRight);
}


void stop() {
  escLeft.writeMicroseconds(1500);
  escRight.writeMicroseconds(1500);
}
// Function to arm the ESC (if necessary)
void armESC() {
  // Send a low signal to the ESC for arming
  Serial.println("Arming ESC...");
  escLeft.writeMicroseconds(map(5, 0, 10, 1000, 2000));
  escRight.writeMicroseconds(map(5, 0, 10, 1000, 2000));
  delay(2000);  // Wait 2 seconds
  Serial.println("ESC armed.");
}
