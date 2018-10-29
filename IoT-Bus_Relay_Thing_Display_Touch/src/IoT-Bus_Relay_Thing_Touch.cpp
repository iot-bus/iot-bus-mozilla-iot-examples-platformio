#include <Arduino.h>
#include <Thing.h>
#include <WebThingAdapter.h>

/*
  Turns on/off a relay using MOZ IoT.
  This example code is in the public domain.
 */

//TODO: Hardcode your wifi credentials here (and keep it private)
const char* ssid = "........";
const char* password = "........";

const int relayPin = 17;  // IoT-Bus relay 16 or 17 supported. If using touchscreen then relay must be on GPIO17 and PENIRQ not used. TFT OK either way.

/* 
 *  Graphics and Touch
 */
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>  // We use modified version that maps from raw to pixel and matches screen orientation

// declare display and touchscreen helpers below
void drawPlug(bool on);
bool touched(bool hit);

//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);//TFT_eSPI display = TFT_eSPI(); // Invoke custom library
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

const int width = 320;
const int height = 240;
#define ILI9341_MOZCYAN  0x0595
#define ILI9341_LIGHTCYAN 0x4DF5

/* Create an instance of the touch screen library */
#define CS_PIN  16 //PENIRQ not used as it would drive relay on same pin

// These are the pins used to interface between the 2046 touch controller and ESP32
// Hardware SPI pins as used above for the display are the same
// DOUT(MISO) 19  Data out pin (T_DO) of touch screen */
// DIN(MOSI)  23  Data in pin (T_DIN) of touch screen */
// DCLK(CLK)  18  Clock pin (T_CLK) of touch screen   */

XPT2046_Touchscreen ts(CS_PIN, 255);  // Param 2 - No interrupts

/* 
 *  MOZ IOT
 */
WebThingAdapter* adapter;

const char* relayTypes[] = {"SmartPlug", nullptr};
ThingDevice relay("Relay", "IoT-Bus Relay", relayTypes);
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

  // draw initial state
  drawPlug(false);

  // start the touch screen
  ts.begin(); 
  
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

ThingPropertyValue value;

void loop() {
  bool on = relayOn.getValue().boolean;
  if(touched(true)){
    on = !on;         // invert status
    lastOn = !on;     // invert status
    value.boolean = on;
    relayOn.setValue(value);
    delay(300); // avoid touch bounce
  }
  // update status
  adapter->update();
  on = relayOn.getValue().boolean;
  digitalWrite(relayPin, on ? HIGH : LOW); // active high
  if (on != lastOn) {
    Serial.print(relay.id);
    Serial.print(": ");
    Serial.println(on);
    drawPlug(on);
  }
  lastOn = on; 
}

// helper to draw the plug state on/off

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
  for (int i=0; i>-5;i--){
    tft.drawCircle(width/2, height/2, RADIUS-i, outlineColor);
  }
  tft.fillCircle(width/2, height/2, RADIUS+1, bgColor);
  
  // sockets
  tft.fillRoundRect(LEFT_SOCKET_X, LEFT_SOCKET_Y, SOCKET_WIDTH, SOCKET_HEIGHT, SOCKET_RADIUS, socketColor);
  tft.fillRoundRect(RIGHT_SOCKET_X, RIGHT_SOCKET_Y, SOCKET_WIDTH, SOCKET_HEIGHT, SOCKET_RADIUS, socketColor);
  tft.fillRoundRect(BOTTOM_SOCKET_X, BOTTOM_SOCKET_Y, SOCKET_WIDTH, SOCKET_HEIGHT, SOCKET_RADIUS, socketColor);

  // We're simply drawing the text because loading fonts is more complex for this example
  // and the base fonts are not smooth

  #define TEXT_O_X 140
  #define TEXT_O_Y 192
  #define TEXT_O_RADIUS 13
 
  #define TEXT_HEIGHT 25
  #define STROKE_WIDTH 5
  
  #define TEXT_2_X 160
  #define TEXT_2_Y 180
  #define TEXT_WIDTH 15

  #define TEXT_3_X 180
  #define TEXT_3_Y 180

  #define OFFSET 8
  
  if(on){
    // O
    tft.fillCircle(TEXT_O_X+OFFSET, TEXT_O_Y, TEXT_O_RADIUS, textColor);
    tft.fillCircle(TEXT_O_X+OFFSET, TEXT_O_Y, TEXT_O_RADIUS-STROKE_WIDTH, bgColor);

    // N
    tft.fillRect(TEXT_2_X+OFFSET, TEXT_2_Y, STROKE_WIDTH, TEXT_HEIGHT, textColor);
    tft.fillRect(TEXT_2_X+TEXT_WIDTH+OFFSET, TEXT_2_Y, STROKE_WIDTH, TEXT_HEIGHT, textColor);
    for(int i=0;i<STROKE_WIDTH;i++){
      tft.drawLine(TEXT_2_X+i+OFFSET, TEXT_2_Y, TEXT_2_X+TEXT_WIDTH+i+OFFSET, TEXT_2_Y+TEXT_HEIGHT, textColor);
    }
  }
  else{

    // O
    tft.fillCircle(TEXT_O_X, TEXT_O_Y, TEXT_O_RADIUS, textColor);
    tft.fillCircle(TEXT_O_X, TEXT_O_Y, TEXT_O_RADIUS-STROKE_WIDTH, bgColor);

    // F
    tft.fillRect(TEXT_2_X, TEXT_2_Y, STROKE_WIDTH, TEXT_HEIGHT, textColor);
    tft.fillRect(TEXT_2_X, TEXT_2_Y, TEXT_WIDTH, STROKE_WIDTH, textColor);
    tft.fillRect(TEXT_2_X, TEXT_2_Y+TEXT_HEIGHT/2, TEXT_WIDTH, STROKE_WIDTH, textColor);

    // F
    tft.fillRect(TEXT_3_X, TEXT_3_Y, STROKE_WIDTH, TEXT_HEIGHT, textColor);
    tft.fillRect(TEXT_3_X, TEXT_3_Y, TEXT_WIDTH, STROKE_WIDTH, textColor);
    tft.fillRect(TEXT_3_X, TEXT_3_Y+TEXT_HEIGHT/2, TEXT_WIDTH, STROKE_WIDTH, textColor);
  } 
}

// returns true if touched in socket image
bool touched(bool hit){
  
  if (ts.touched()) 
  {
    Serial.println("touched");
    // Read the current X and Y axis as co-ordinates at the last touch time
    TS_Point p = ts.getMappedPoint();
    Serial.print(p.x);Serial.print(" ");Serial.println(p.y);Serial.print(" ");Serial.println(p.z);
    if(sqrt((p.x-width/2)*(p.x-width/2) + (p.y-height/2)*(p.y-height/2)) < RADIUS){
      Serial.println("Hit");
      return true;
    }
  }
  return false;
}
