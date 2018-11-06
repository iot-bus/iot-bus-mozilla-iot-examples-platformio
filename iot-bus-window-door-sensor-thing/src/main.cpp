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

// Uses a wondow/door sensor and turns on a LED when open
int ledPin = 5;    // choose the pin for the LED
int sensorPin = 4; // choose the input pin - we'll use a pullup on this pin to keep high
                   // Connect one end of the door switch to this pin. 
                   // Connect the other end to GND

WebThingAdapter* adapter;

const char* sensorTypes[] = {"binarySensor", nullptr};
ThingDevice sensor("DoorSensor", "Window/Door Sensor", sensorTypes);
ThingProperty openProperty("Sensor Open", "", BOOLEAN, "OpenProperty", "Door", "", false);
ThingPropertyValue sensorValue;

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
  sensor.addProperty(&openProperty);
  adapter->addDevice(&sensor);
  adapter->begin();

  pinMode(sensorPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT); // declare LED as output
  
}

void loop() {

  int val = digitalRead(sensorPin); // get value 

  Serial.println(val);  
  if (val == LOW){
     sensorValue.boolean = false; // pulled to GND
     digitalWrite(ledPin, LOW);
  }
  else{
     sensorValue.boolean = true; // pullup active
     digitalWrite(ledPin, HIGH);
  }
  openProperty.setValue(sensorValue);
  adapter->update();
  delay(300);
}
