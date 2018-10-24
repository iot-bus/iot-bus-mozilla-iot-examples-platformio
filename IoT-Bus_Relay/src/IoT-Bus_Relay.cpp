#include <Arduino.h>
/*

  Turns on a relay on for three seconds, then off for three seconds, repeatedly.

  This example code is in the public domain.

 */
#define RELAYPIN 17

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 16 as an output.
  pinMode(RELAYPIN, OUTPUT);
  Serial.begin(115200);
}

// the loop function runs over and over again forever
void loop() {
  Serial.println("HIGH");
  digitalWrite(RELAYPIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(3000); 
  Serial.println("LOW");// wait for three seconds
  digitalWrite(RELAYPIN, LOW);    // turn the LED off by making the voltage LOW
  delay(3000);              // wait for three seconds
}
