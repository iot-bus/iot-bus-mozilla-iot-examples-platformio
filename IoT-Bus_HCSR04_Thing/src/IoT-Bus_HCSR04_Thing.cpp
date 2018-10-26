/* 
 *  Arduino framework 
 */

#include <arduino.h>

/* 
 *  MOZ IoT 
 */
#include <Thing.h>
#include <WebThingAdapter.h>

/* 
 *  Graphics 
 */
#include "TFT_eSPI.h" // Hardware-specific library

TFT_eSPI display = TFT_eSPI();

WebThingAdapter* adapter;

const char* sensorTypes[] = {"LevelSensor", nullptr};
ThingDevice hcsr04("HC-SR04", "HC-SR04", sensorTypes);
ThingProperty distance("distance", "Distance in cm", NUMBER, "LevelProperty", "Distance", "in", "false");
ThingPropertyValue measurement;

/*
 *  WiFi ssid and password
 */
const char* ssid = ".........";
const char* password = "........";

/*
 *  HC-SR04 
 */
int trigPin = 2;    // Trigger
int echoPin = 4;    // Echo
long duration, cm, inches;

/*
 *  displayString helper function to draw text on 
 *  the TFT display
 */
const int textHeight = 18;
const int textWidth = 18;
const int width = 320;
const int height = 240;

String last, current; // current and last values of text

void displayString(const String& str, int color) {
  int len = str.length()+1;
  int strWidth = len * textWidth;
  int strHeight = textHeight;
  int scale = width / strWidth;

  int x = width / 2 - (strWidth * scale / 2);
  int y = height / 2 + (strHeight * scale / 2);

  display.setFreeFont(&FreeSans18pt7b);
  display.setRotation(1);
  display.setTextColor(color);
  display.setTextSize(scale);
  display.setCursor(x, y);
  display.println(str);
  Serial.println(str);
 }

/*
 *  First-time initialization   
 */
 
void setup() {
  // Start serial monitor - make sure same speed as monitor
  Serial.begin (115200);
  
  // HC-SR04 pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Turn on display
  pinMode(33, OUTPUT); // turn on the IoT-Bus Display
  digitalWrite(33, HIGH);
  
  // Start display and clear
  display.begin();
  display.fillScreen(ILI9341_BLACK);

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
  adapter = new WebThingAdapter("textdisplayer", WiFi.localIP());
  measurement.number = -1;
  distance.setValue(measurement);
  hcsr04.addProperty(&distance);
  adapter->addDevice(&hcsr04);
  adapter->begin();
}
 
void loop() {
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
 
  // Convert the time into a distance
  cm = (duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  inches = (duration/2) / 74;   // Divide by 74 or multiply by 0.0135

// Uncomment to display on serial monitor  
//  Serial.print(inches);
//  Serial.print("in, ");
//  Serial.print(cm);
//  Serial.print("cm");
//  Serial.println();
  if(cm >= 400 || cm < 2){
    current = "Out of range";
  }
  else{
    current = String(inches) + " in"; 
  }
  if (current != last){
    displayString(last, ILI9341_BLACK);    // clear old text by writing it black
    displayString(current, ILI9341_WHITE); // write the new value
    measurement.number = inches;
    distance.setValue(measurement);
    adapter->update();                     // update the MOZ IoT thing
    last = current;                        // remember the last write to be able to clear it
    delay(500);                            // vary to suit
   }
}
