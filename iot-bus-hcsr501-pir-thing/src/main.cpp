#include <Arduino.h>

/*
  Simple motion sensor example using HC-S501 
  This example code is in the public domain.
 */

#include <Arduino.h>
#include <Thing.h>
#include <WebThingAdapter.h>

//TODO: Hardcode your wifi credentials here (and keep it private)
const char* ssid = "........";
const char* password = "........";

// Uses a PIR sensor to detect movement and turn on a LED
int ledPin = 5; // choose the pin for the LED
int inputPin = 4; // choose the input pin (for PIR sensor)

WebThingAdapter* adapter;

const char* sensorTypes[] = {"MotionSensor", nullptr};
ThingDevice hcsr501("HC-SR501", "HC-SR501 PIR Sensor", sensorTypes);
ThingProperty sensorOn("on", "", BOOLEAN, "MotionProperty");
ThingPropertyValue sensorValue;

void setup() {

  Serial.begin(115200); //Use serial monitor for debugging

  // Start WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize MOZ IoT thing
  adapter = new WebThingAdapter("adapter", WiFi.localIP());
  hcsr501.addProperty(&sensorOn);
  adapter->addDevice(&hcsr501);
  adapter->begin();

  pinMode(ledPin, OUTPUT); // declare LED as output
  pinMode(inputPin, INPUT); // declare sensor as input
}

void loop(){
  int val = digitalRead(inputPin); // read input value
  if (val == HIGH) { // check if the input is HIGH
    if (sensorOn.getValue().boolean != true) {
      // Turned on
      Serial.println("Motion detected!");
      // Update on the output change, not state
      sensorValue.boolean = true;
      digitalWrite(ledPin, HIGH);
    }
  } else {
    if (sensorOn.getValue().boolean == true){
      // Turned off
      Serial.println("Motion ended!");
      // Update on the output change, not state
      sensorValue.boolean = false;
      digitalWrite(ledPin, LOW);
    }
  }
  // update the adapter status
  sensorOn.setValue(sensorValue);
  adapter->update();
  delay(300); // need to retain state long enough to be visible
}