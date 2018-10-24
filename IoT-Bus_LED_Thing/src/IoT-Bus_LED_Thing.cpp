/**
 * Simple server compliant with Mozilla's proposed WoT API
 * Originally based on the HelloServer example
 * Tested on ESP8266, ESP32, Arduino boards with WINC1500 modules (shields or
 * MKR1000)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <Arduino.h>
#include "Thing.h"
#include "WebThingAdapter.h"

//TODO: Hardcode your wifi credentials here (and keep it private)
const char* ssid = "NETGEAR96";
const char* password = "phoebe1984";

#if defined(LED_BUILTIN)
const int ledPin = LED_BUILTIN;
#else
const int ledPin = 16;  // manually configure LED pin
#endif

WebThingAdapter* adapter;

const char* relayTypes[] = {"OnOffSwitch", "Relay", nullptr};
ThingDevice relay("relay", "Relay", relayTypes);
ThingProperty relayOn("on", "", BOOLEAN, "OnOffProperty");

bool lastOn = false;

void setup(void){
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  Serial.begin(115200);
  Serial.println("");
  Serial.print("Connecting to \"");
  Serial.print(ssid);
  Serial.println("\"");
#if defined(ESP8266) || defined(ESP32)
  WiFi.mode(WIFI_STA);
#endif
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  bool blink = true;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(ledPin, blink ? LOW : HIGH); // active low led
    blink = !blink;
  }
  digitalWrite(ledPin, HIGH); // active low led

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  adapter = new WebThingAdapter("w25", WiFi.localIP());

  relay.addProperty(&relayOn);
  adapter->addDevice(&relay);
  adapter->begin();
  Serial.println("HTTP server started");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.print("/things/");
  Serial.println(relay.id);
}

void loop(void){
  adapter->update();
  bool on = relayOn.getValue().boolean;
  digitalWrite(ledPin, on ? HIGH : LOW); // active high led
  if (on != lastOn) {
    Serial.print(relay.id);
    Serial.print(": ");
    Serial.println(on);
  }
  lastOn = on;
}
