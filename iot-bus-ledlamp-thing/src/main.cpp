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
#include <Thing.h>
#include <WebThingAdapter.h>

#define MIN(a,b) (((a)<(b))?(a):(b))

const char* ssid = "........";
const char* password = "........";

#if defined(LED_BUILTIN)
const int lampPin = LED_BUILTIN;
#else
const int lampPin = 5;  // manully configure LED pin
#endif

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

// use 13 bit precision for LEDC timer
#define LEDC_TIMER_13_BIT  13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     5000

WebThingAdapter* adapter;

const char* lampTypes[] = {"OnOffSwitch", "Light", nullptr};
ThingDevice lamp("lamp", "My Lamp", lampTypes);

ThingProperty lampOn("on", "Whether the lamp is turned on", BOOLEAN, "OnOffProperty");
ThingProperty lampLevel("level", "The level of light from 0-100", NUMBER, "BrightnessProperty");

// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * MIN(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}

void setup(void){
  pinMode(lampPin, OUTPUT);
  digitalWrite(lampPin, LOW); // initially off

  // Setup timer and attach timer to a led pin
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(lampPin, LEDC_CHANNEL_0);
  
  Serial.begin(115200);
#if defined(ESP8266) || defined(ESP32)
  WiFi.mode(WIFI_STA);
#endif
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
  adapter = new WebThingAdapter("led-lamp", WiFi.localIP());

  lamp.addProperty(&lampOn);
  lamp.addProperty(&lampLevel);
  adapter->addDevice(&lamp);
  adapter->begin();
  Serial.println("HTTP server started");

#ifdef analogWriteRange
  analogWriteRange(255);
#endif
}

void loop(void){
  adapter->update();
  if (lampOn.getValue().boolean) {
    int level = map(lampLevel.getValue().number, 0, 100, 0, 255);
//    Serial.println(lampLevel.getValue().number);
    ledcAnalogWrite(LEDC_CHANNEL_0, level);

//    analogWrite(lampPin, level);
  } else {
    ledcAnalogWrite(LEDC_CHANNEL_0, 0);
 //   analogWrite(lampPin, 255);
  }
}