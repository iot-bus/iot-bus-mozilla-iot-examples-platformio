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

const int relayPin = 17;  // IoT-Bus relay 16 or 17 supported. If using touchscreen then relay must be on GPIO17 and PENIRQ not used. TFT OK either way.

/* 
 *  Graphics  
 */
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // Hardware-specific library
#include <Fonts/FreeSansBold18pt7b.h>

void drawPlug(bool on);

#define TFT_CS 5     // CS for display
#define TFT_MISO 19  // VSPI MISO
#define TFT_MOSI 23  // VSPI MOSI
#define TFT_CLK 18   // VSPI CLOCK
#define TFT_DC 27    // Data/command pin for display
#define TFT_RST 0    // no RESET pin

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);//TFT_eSPI display = TFT_eSPI(); // Invoke custom library

const int width = 320;
const int height = 240;
#define ILI9341_MOZCYAN  0x0595
#define ILI9341_LIGHTCYAN 0x4DF5

/* 
 *  MOZ IOT
 */
WebThingAdapter* adapter;

const char* relayTypes[] = {"SmartPlug", nullptr};
ThingDevice relay("relay", "IoT-Bus Relay", relayTypes);
ThingProperty relayOn("on", "", BOOLEAN, "OnOffProperty");

// remember last state
bool lastOn = false;

// the setup function runs once when you press reset or power the board

void setup() {
  Serial.begin(115200); //Use serial monitor for debugging

  // initialize relay pin as an output.
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  //Start WiFi
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

  // Turn on display
  pinMode(33, OUTPUT); // turn on the IoT-Bus Display
  digitalWrite(33, HIGH);
  
  // Start display and draw plug
  tft.begin();
  tft.setRotation(1);
   // background
  tft.fillScreen(ILI9341_MOZCYAN); // closest 16 bit color to MOZ IOT
 
  drawPlug(false);

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
    drawPlug(on);
  }
  lastOn = on; 
}

// draw the plus state on/off

void drawPlug(bool on=false){
  #define RADIUS 110
  #define SOCKET_WIDTH 24
  #define SOCKET_HEIGHT 40
  #define SOCKET_RADIUS 10
  #define LEFT_SOCKET_X 108
  #define LEFT_SOCKET_Y 55
  #define RIGHT_SOCKET_X 185
  #define RIGHT_SOCKET_Y 55
  #define BOTTOM_SOCKET_X 147
  #define BOTTOM_SOCKET_Y 115

  uint16_t bgColor;
  uint16_t socketColor;
  uint16_t textColor;
  uint16_t outlineColor;
  String text;

  if(on){
    bgColor = ILI9341_WHITE;
    outlineColor = ILI9341_WHITE;
    socketColor = ILI9341_MOZCYAN;
    textColor = ILI9341_MOZCYAN;
    text = " ON";
  }
  else{
    bgColor = ILI9341_LIGHTCYAN;
    outlineColor = ILI9341_WHITE;
    socketColor = ILI9341_WHITE;
    textColor = ILI9341_WHITE;
    text = "OFF";
  }
 
  // outline
  //tft.drawCircle(width/2, height/2, RADIUS+1, ILI9341_LIGHTGREY);
  for (int i=0; i>-5;i--){
    tft.drawCircle(width/2, height/2, RADIUS-i, outlineColor);
  }
  tft.fillCircle(width/2, height/2, RADIUS+1, bgColor);
  
  // sockets
  tft.fillRoundRect(LEFT_SOCKET_X, LEFT_SOCKET_Y, SOCKET_WIDTH, SOCKET_HEIGHT, SOCKET_RADIUS, socketColor);
  tft.fillRoundRect(RIGHT_SOCKET_X, RIGHT_SOCKET_Y, SOCKET_WIDTH, SOCKET_HEIGHT, SOCKET_RADIUS, socketColor);
  tft.fillRoundRect(BOTTOM_SOCKET_X, BOTTOM_SOCKET_Y, SOCKET_WIDTH, SOCKET_HEIGHT, SOCKET_RADIUS, socketColor);

  #define TEXT_X 125
  #define TEXT_Y 200
  
  // on/off
  tft.setTextColor(textColor);
  tft.setFont(&FreeSansBold18pt7b);
  tft.setCursor(TEXT_X, TEXT_Y);
  tft.println(text);
}
