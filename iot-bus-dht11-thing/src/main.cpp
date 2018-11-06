#include <arduino.h>
/* 
 *  Graphics  
 */
#include <TFT_eSPI.h>

TFT_eSPI display = TFT_eSPI();

/* 
 *  MOZ IoT 
 */
#include <Thing.h>
#include <WebThingAdapter.h>

WebThingAdapter* adapter;

const char* sensorTypes[] = {"MultiLevelSensor", nullptr};

ThingDevice dht11("thermometer", "DHT11 Thermometer & Hygrometer", sensorTypes);
// ThingProperty variable_name( id, description, type, @type, label, unit, writable );  // writable not supported until 0.6

ThingProperty temperature("temperature", "The temperature from x to y", NUMBER, "LevelProperty", "Temperature", "F", false);
ThingProperty humidity("humidity", "The humidity from 0 to 100%", NUMBER, "LevelProperty", "Humidity", "percent", false);

ThingPropertyValue reading;

/*
 *  WiFi ssid and password
 */
const char* ssid = "........";
const char* password = "........";

/*
 *  DHT11
 */
#include "DHTesp.h"

DHTesp dht;

/** Comfort profile */
ComfortState cf;

int dhtPin = 4;

/**
 * getTemperature
 * Reads temperature from DHT11 sensor
 * @return bool
 *    true if temperature could be aquired
 *    false if aquisition failed
*/

TempAndHumidity newValues;

bool getTemperature() {
	// Reading temperature for humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  newValues = dht.getTempAndHumidity();
	// Check if any reads failed and exit early (to try again).
	if (dht.getStatus() != 0) {
		Serial.println("DHT11 error status: " + String(dht.getStatusString()));
		return false;
	}

	float heatIndex = dht.computeHeatIndex(newValues.temperature, newValues.humidity);
  float dewPoint = dht.computeDewPoint(newValues.temperature, newValues.humidity);
  float cr = dht.getComfortRatio(cf, newValues.temperature, newValues.humidity);

  String comfortStatus;
  switch(cf) {
    case Comfort_OK:
      comfortStatus = "Comfort_OK";
      break;
    case Comfort_TooHot:
      comfortStatus = "Comfort_TooHot";
      break;
    case Comfort_TooCold:
      comfortStatus = "Comfort_TooCold";
      break;
    case Comfort_TooDry:
      comfortStatus = "Comfort_TooDry";
      break;
    case Comfort_TooHumid:
      comfortStatus = "Comfort_TooHumid";
      break;
    case Comfort_HotAndHumid:
      comfortStatus = "Comfort_HotAndHumid";
      break;
    case Comfort_HotAndDry:
      comfortStatus = "Comfort_HotAndDry";
      break;
    case Comfort_ColdAndHumid:
      comfortStatus = "Comfort_ColdAndHumid";
      break;
    case Comfort_ColdAndDry:
      comfortStatus = "Comfort_ColdAndDry";
      break;
    default:
      comfortStatus = "Unknown:";
      break;
  };

  Serial.println(" T:" + String(newValues.temperature) + " H:" + String(newValues.humidity) + " I:" + String(heatIndex) + " D:" + String(dewPoint) + " " + comfortStatus);
	return true;
}

/*
 *  displayString helper function to draw text on 
 *  the TFT display
 */
const int textHeight = 12;
const int textWidth = 12;
const int width = 320;
const int height = 240;

String last, current; // current and last values of text

void displayString(const String& str, int color) {
  int len = str.length()+1;
  int strWidth = len * textWidth;
  int strHeight = textHeight;
  Serial.println(strWidth);
  int scale = width / strWidth;
  Serial.println(scale);
  if (scale < 1) 
    scale = 1;
  
  int x = width / 2 - strWidth * scale / 2;
  int y = height / 2 + strHeight * scale / 2;

  display.setFreeFont(&FreeSans18pt7b);
  display.setRotation(1);
  display.setTextColor(color);
  display.setTextSize(scale);
  display.setCursor(x, y);
  display.println(str);
 }


void setup()
{
  Serial.begin(115200);
  dht.setup(dhtPin, DHTesp::DHT11);

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
  adapter = new WebThingAdapter("multilevelsensor", WiFi.localIP());
  dht11.addProperty(&temperature);
  dht11.addProperty(&humidity);
  adapter->addDevice(&dht11);
  adapter->begin();
}

void loop() {
  getTemperature();
  current = String(dht.toFahrenheit(newValues.temperature)) + "°F   " + String(newValues.humidity) + "%";
  if (current != last){
    displayString(last, ILI9341_BLACK);    // clear old text by writing it black
    displayString(current, ILI9341_WHITE); // write the new value
    reading.number = dht.toFahrenheit(newValues.temperature);  // needs to be a PropertyValue
    temperature.setValue(reading);         // now set the property
    reading.number = newValues.humidity;   // needs to be a PropertyValue
    humidity.setValue(reading);            // now set the property
    adapter->update();                     // update the MOZ adapter
    last = current;                        // remember the last write to be able to clear it
   } 
   delay(1200);
}
