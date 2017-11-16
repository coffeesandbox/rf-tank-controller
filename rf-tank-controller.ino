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

  // Random seed
  randomSeed(analogRead(0));

  Serial.println("Initialized");
}

void loop() {    
  stateUpdate.motorLeft = random(60, 77);
  stateUpdate.motorRight = random(60, 77);
  
  radio.stopListening();
  if (radio.write(&stateUpdate, sizeof(stateUpdate))) {
      printf("State update: Left [%d], Right [%d]\n", stateUpdate.motorLeft, stateUpdate.motorRight);
  }

  radio.startListening();
  
  delay(5000);
}
