#include <Arduino.h>
#include <Thing.h>
#include <WebThingAdapter.h>

/*
  Simple binary sensor example using ESP32 capacitive touch input
  This example code is in the public domain.
 */

//TODO: Hardcode your wifi credentials here (and keep it private)
const char* ssid = "........";
const char* password = "........";

// Uses a touch sensor to detect input and turn on a LED
int ledPin = 5; // choose the pin for the LED
int touchPin = 4; // choose the input pin - T0 is the same as GPIO4

WebThingAdapter* adapter;

const char* sensorTypes[] = {"binarySensor", nullptr};
ThingDevice touch("Touch", "ESP32 Touch Input", sensorTypes);
ThingProperty touched("true", "", BOOLEAN, "BooleanProperty");
ThingPropertyValue sensorValue;

int threshold = 40;

void setup() {
  Serial.begin(115200);

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
  touch.addProperty(&touched);
  adapter->addDevice(&touch);
  adapter->begin();

  pinMode(ledPin, OUTPUT); // declare LED as output
  
}

void loop() {

  int val = touchRead(T0); // get value using T0 / GPIO4

  Serial.println(val);  
  if (val < threshold){
     sensorValue.boolean = true;
     digitalWrite(ledPin, HIGH);
  }
  else{
     sensorValue.boolean = false;
     digitalWrite(ledPin, LOW);
  }
  touched.setValue(sensorValue);
  adapter->update();
  delay(300);
}
