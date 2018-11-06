#include <Arduino.h>
#include "Thing.h"
#include "WebThingAdapter.h"

/*

  Turns on/off a relay using MOZ IoT.

  This example code is in the public domain.

 */

//TODO: Hardcode your wifi credentials here (and keep it private)
const char* ssid = "........";
const char* password = "........";

const int relayPin = 17;  // IoT-Bus relay

WebThingAdapter* adapter;

const char* relayTypes[] = {"SmartPlug", nullptr};
ThingDevice relay("relay", "IoT-Bus Relay", relayTypes);
ThingProperty relayOn("on", "", BOOLEAN, "OnOffProperty");

bool lastOn = false;

// the setup function runs once when you press reset or power the board

void setup() {
  Serial.begin(115200); //Use serial monitor for debugging

  // initialize relay pin as an output.
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

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
  relay.addProperty(&relayOn);
  adapter->addDevice(&relay);
  adapter->begin();
  Serial.println("HTTP server started");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.print("/things/");
  Serial.println(relay.id);
}

void loop() {
  // update status
  adapter->update();
  bool on = relayOn.getValue().boolean;
  digitalWrite(relayPin, on ? HIGH : LOW); // active high
  if (on != lastOn) {
    Serial.print(relay.id);
    Serial.print(": ");
    Serial.println(on);
  }
  lastOn = on; 
}