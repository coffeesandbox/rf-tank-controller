#include <printf.h>
#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>

// ManiacBug RF24 lib is found here
// https://github.com/maniacbug/RF24

RF24 radio(53, 48); // CE, CSN
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

typedef struct stateUpdate {
  int controlBit = 1;
  int motorLeft;
  int motorRight;
} StateUpdate;
 
StateUpdate stateUpdate;

int leftStick = A0;   // Down-Up
int rightStick = A1;  // Left-Right
int controlPot = A2;  // RightMotor Sync (might be needed to sync motor speed)

void setup() {  
  // Initialize serial communication
  Serial.begin(57600);
  Serial.println("System initializing");
  printf_begin();

  // Initialize radio module
  radio.begin();
  radio.setRetries(15,15);
  radio.openWritingPipe(pipes[0]);
  radio.startListening();
  radio.printDetails();

  // Initialize analog inputs
  pinMode(leftStick,   INPUT);
  pinMode(rightStick,   INPUT);
  pinMode(controlPot,  INPUT);

  Serial.println("Initialized");
}

void computeMotorsSpeed() {
  int leftStickValue = analogRead(leftStick);

  if (leftStickValue < 490) {
    stateUpdate.motorRight = map(leftStickValue, 500, 350, 0, 100); //86->55
    stateUpdate.motorLeft = map(leftStickValue, 500, 350, 0, 100); 
  } else if (leftStickValue > 510) {
    stateUpdate.motorRight = map(leftStickValue, 660, 500, 0, 100);
    stateUpdate.motorLeft = map(leftStickValue, 660, 500, 0, 100); 
  } else {
    stateUpdate.motorRight = 86;
    stateUpdate.motorLeft = 86;
  }

  int rightStickValue = analogRead(rightStick); 
  int directionAdjustment = map(rightStickValue, 330, 680, 0, 100);

  printf("1. L [%d] R [%d] A [%d] \n",stateUpdate.motorLeft, stateUpdate.motorRight, directionAdjustment);

  if (directionAdjustment >= 48) {
    directionAdjustment = map(directionAdjustment, 48, 100, 0, 100);
    stateUpdate.motorLeft = stateUpdate.motorLeft - (stateUpdate.motorLeft * directionAdjustment) / 100;
  } else if (directionAdjustment <= 44) {
    directionAdjustment = map(directionAdjustment, 0, 44, 100, 0);
    stateUpdate.motorRight = stateUpdate.motorRight - ((stateUpdate.motorRight * directionAdjustment) / 100);
  }

  printf("2. L [%d] R [%d] A [%d] \n",stateUpdate.motorLeft, stateUpdate.motorRight, directionAdjustment);

  if (leftStickValue < 490) {
    stateUpdate.motorRight = map(stateUpdate.motorRight, 0, 100, 86, 55); //86->55
    stateUpdate.motorLeft = map(stateUpdate.motorLeft, 0, 100, 86, 55); 
  } else if (leftStickValue > 510) {
    stateUpdate.motorRight = map(stateUpdate.motorRight, 0, 100, 120, 100);
    stateUpdate.motorLeft = map(stateUpdate.motorLeft, 0, 100, 120, 100); 
  } else {
    stateUpdate.motorRight = 86;
    stateUpdate.motorLeft = 86;
  }

  printf("3. L [%d] R [%d] A [%d] \n",stateUpdate.motorLeft, stateUpdate.motorRight, directionAdjustment);
}

void loop() {    
  int controlPotValue = analogRead(controlPot);

  computeMotorsSpeed();
  
  radio.stopListening();
  if (radio.write(&stateUpdate, sizeof(stateUpdate))) {
      printf("State update: Left [%d], Right [%d], ControlPot [%d]\n", stateUpdate.motorLeft, stateUpdate.motorRight, controlPotValue);
  }
  radio.startListening();
  
  delay(50);
}
