
/*______Import Libraries_______*/
#include "TFT_eSPI.h" // Hardware-specific library
#include "colors.h" // extended colors
#include <XPT2046_Touchscreen.h>
#include <FS.h>
#include <WiFi.h>
/* 
 *  MOZ IoT 
 */
#include <Thing.h>
#include <WebThingAdapter.h>
/*______End of Libraries_______*/

/*
 *  WiFi ssid and password
 */
const char* ssid = "........";
const char* password = "........";

// simple window style class used for the response box and buttons
enum alignment { LEFT, RIGHT, MIDDLE };

class Button{
public:
  Button(TFT_eSPI& _tft): tft(_tft){}
  void setRect(uint16_t _x, uint16_t _y, uint16_t _width, uint16_t _height){
    x = _x;
    y = _y;
    width = _width;
    height =_height;
  }
  void setText(String _text, uint16_t _color){text = _text;color = _color;}
  String& getText(){return text;}
  void setType(uint8_t _type){type = _type;}
  uint8_t getType(){return type;}
  void setBgColor(uint16_t _bgColor){bgColor = _bgColor;}
  void setTextAlign(uint8_t _alignment){alignment = _alignment;}
  void draw(){
    tft.fillRect(x, y, width, height, bgColor);
    switch(alignment){
      default:
      case MIDDLE:
        tft.setCursor(x+width/3, y+height/3); // center text
        break;
      case LEFT:
        tft.setCursor(x+10, y+height/3);
        break;
      case RIGHT:
        tft.setCursor(x+width, y+height/3);
        break;     
    }
    // draw the outline
    tft.drawFastHLine(x, y, width, TFT_WHITE);
    tft.fillRect(x, y+height-5, width, 5, TFT_DARKGREY);
    tft.drawFastHLine(x, y+height-1, width, TFT_WHITE);
    tft.drawFastVLine(x, y, height, TFT_WHITE);
    tft.fillRect(x+width-5, y, 5, height, TFT_DARKGREY);
    tft.drawFastVLine(x+width-1, y, height, TFT_WHITE);
    
    // draw the text
    tft.setTextSize(4);
    tft.setTextColor(color);
    tft.println(text); 
  }
  bool hit(TS_Point p){
    if (p.x<x+width && p.x>x){
        if (p.y<y+height && p.y>y){
          // draw the outline as pushed
          tft.drawFastHLine(x, y, width, TFT_WHITE);
          tft.fillRect(x, y, width, 5, TFT_DARKGREY);
          tft.drawFastHLine(x, y+height-1, width, TFT_WHITE);
          tft.drawFastVLine(x, y, height, TFT_WHITE);
          tft.fillRect(x, y, 5, height, TFT_DARKGREY);
          tft.drawFastVLine(x+width-1, y, height, TFT_WHITE);
          delay(300); // enough time to be visible
          draw(); // redraw
          return true;
        }
    }
    return false;
  }

private:
  TFT_eSPI& tft;
	uint16_t x = 0, y = 0, width = 240, height = 320, color = TFT_WHITE, bgColor = TFT_BLACK;
  uint8_t type; // user type
  uint8_t alignment = MIDDLE;
  String text = "";
};

/*____Calibration values TFT TS_____*/
#define TS_MINX 256
#define TS_MINY 274
#define TS_MAXX 3632
#define TS_MAXY 3579
/*______End of Calibration______*/

class Calculator{
  private:
    TFT_eSPI& tft;
    XPT2046_Touchscreen ts;

    String symbol[4][4] = {
      { "7", "8", "9", "/" },
      { "4", "5", "6", "*" },
      { "1", "2", "3", "-" },
      { "C", "0", "=", "+" }
    };

    unsigned int buttonColors[4][4] = {
      { TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTBLUE },
      { TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTBLUE },
      { TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTBLUE },
      { TFT_LIGHTSALMON, TFT_LIGHTGREY, TFT_LIGHTGREEN, TFT_LIGHTBLUE }
    };

    enum buttonType{ NUM, CLEAR, EQUALS, DIVIDE, MULTIPLY, ADD, SUBTRACT };

    uint8_t buttonTypes[4][4] = {
      { NUM, NUM, NUM, DIVIDE },
      { NUM, NUM, NUM, MULTIPLY },
      { NUM, NUM, NUM, SUBTRACT },
      { CLEAR,  NUM, EQUALS, ADD }
    };

    #define BUTTONCOUNT 16
    Button* buttons[BUTTONCOUNT];

    long Num1,Num2,Accumulator;
    char action;

    Button* resultBox;
    String function = "";
  
  public:
    Calculator(TFT_eSPI& _tft, XPT2046_Touchscreen _ts): tft(_tft), ts(_ts){
      // initialize display
      tft.init(); 
      tft.setRotation(0); 
      ts.setRotation(0);
      ts.setCalibration(TS_MINX, TS_MAXX, TS_MINY, TS_MAXY);
    }

    long getAccumulator(){return Accumulator;}
    long getNum1(){return Num1;}
    long getNum2(){return Num2;}
    String getFunction(){return function;}

    // wait for a button press
    TS_Point waitTouch() {
      while (!ts.touched()){
          // do nothing
      }
      Serial.println("touched");
        // Read the current X and Y axis as co-ordinates at the last touch time
        // The values were captured when Pressed() was called!
      TS_Point p = ts.getPoint();
      Serial.println("--------------------------------");
      // print raw data
      Serial.print("raw:");Serial.print(p.x);Serial.print(",");Serial.println(p.y);
      p = ts.getMappedPoint();
      // print data mapped to width and height of TFT depending on rotation
      Serial.print("mapped by library:");Serial.print(p.x);Serial.print(",");Serial.println(p.y);
      Serial.println("--------------------------------");  
      return p;
    }

    void draw()
    {
      #define resultBoxHeight 80
      #define buttonHeight 60
      #define buttonWidth 60

      resultBox = new Button(tft);
    
      // we are using the touchscreen to getwidth as we know it changes with orientation - need to check tft
      resultBox->setRect(0, 0, ts.getWidth(), resultBoxHeight);
      resultBox->setBgColor(TFT_BLACK);
      resultBox->setText(String("IoT-Bus"), TFT_WHITE);
      resultBox->setTextAlign(LEFT);
      resultBox->draw();

      // Create keypad
      for (int j=0;j<4;j++) {
        for (int i=0;i<4;i++) {
          buttons[(j*4)+i] = new Button(tft);
          buttons[(j*4)+i]->setRect( 60*i, resultBoxHeight + (60*j), buttonWidth, buttonHeight );
          buttons[(j*4)+i]->setBgColor(buttonColors[j][i]);
          buttons[(j*4)+i]->setText(String(symbol[j][i]), TFT_BLACK);
          buttons[(j*4)+i]->setType(buttonTypes[j][i]);  
          buttons[(j*4)+i]->draw();
        }
      }
    }

    // Handle each button
    void handleButton(Button* button){
      uint8_t type = button->getType();
      Serial.println(type);
      switch(type){
        default:
        case NUM:
          handleNumber(button);
          displayResult();
          Serial.println("number");
          break;
        case CLEAR:
          Serial.println("clear");
          Accumulator=Num1=Num2=0;
          displayResult(); 
          break;
        case EQUALS:
          handleEquals(button);
          displayResult();
          Serial.println("=");
          break;
        case MULTIPLY:
        case DIVIDE:
        case ADD:
        case SUBTRACT:
          action = button->getType();
          Serial.println("function");
          Num1 = Accumulator;    
          Accumulator = 0;
          function = button->getText();
          break;
      }
    }
    // Hittest for all buttons
    Button* hitTest(TS_Point p){
      for(int i=0;i<BUTTONCOUNT;i++){
        if(buttons[i]->hit(p)){
          Serial.println(buttons[i]->getText());
          handleButton(buttons[i]);
          return buttons[i];
        }
      }
      return nullptr;
    }

    void displayResult(){
    resultBox->setText(String(Accumulator), TFT_WHITE);
    resultBox->draw();
}

    // Handle number
    void handleNumber(Button* button){   
      if (Accumulator==0)
        Accumulator= button->getText().toInt(); // First press
      else
        Accumulator = (Accumulator*10) + button->getText().toInt(); // Second press
    }

    // Handle =
    void handleEquals(Button* button){
      Num2=Accumulator;
      switch(action){
        default:
        case ADD:
          Accumulator = Num1+Num2;
          Serial.println("+");
          break;
        case SUBTRACT:
          Accumulator = Num1-Num2;
          Serial.println("-");
          break;  
        case MULTIPLY:
          Accumulator = Num1*Num2;
          Serial.println("*");
          break;  
        case DIVIDE:
          Accumulator = Num1/Num2;
          Serial.println("/");
          break;  
      }
    }
};

// Globals

Calculator* calculator;

// MOZ IoT
WebThingAdapter* adapter;
ThingPropertyValue value;
String iotbus ("IoT-Bus");
String accumulator, num1, num2;

const char* textDisplayTypes[] = {"TextDisplay", nullptr};
ThingDevice textDisplay("textDisplay", "IoT-Bus Calculator", textDisplayTypes);
ThingProperty accumulatorProperty("Accumulator", "", NUMBER, nullptr);
ThingProperty num1Property("Number 1", "", NUMBER, nullptr);
ThingProperty num2Property(" Number 2", "", NUMBER, nullptr);
ThingProperty functionProperty("Last Function", "", STRING, nullptr);

// Create TFT
TFT_eSPI tft = TFT_eSPI();   

// Create Touchscreen
#define CS_PIN 16 // touch pin CS
XPT2046_Touchscreen ts(CS_PIN, 255); // No IRQ Pin

void setup() {
  Serial.begin(115200); //Use serial monitor for debugging

  Serial.println("Starting Caclulator");
  ts.setSize(240,320); // set width, height
  ts.begin();
 
  // Turn on display
  pinMode(33, OUTPUT); // turn on the IoT-Bus Display
  digitalWrite(33, HIGH);

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
  value.string = &iotbus;
  accumulatorProperty.setValue(value);
  textDisplay.addProperty(&accumulatorProperty);
  textDisplay.addProperty(&num1Property);
  textDisplay.addProperty(&num2Property);
  functionProperty.setValue(value);
  textDisplay.addProperty(&functionProperty);
  adapter->addDevice(&textDisplay);
  adapter->begin();

  // Create and draw calculator
  calculator = new Calculator(tft, ts);
  calculator->draw(); 
}

String function; // needs to be around for MOZ IoT

void loop() {
  // wait for a button press
  TS_Point p = calculator->waitTouch();
  calculator->hitTest(p);

  // update MOZ IoT 
  value.number = calculator->getAccumulator();
  accumulatorProperty.setValue(value);
  value.number = calculator->getNum1();
  num1Property.setValue(value);
  value.number = calculator->getNum2();
  num2Property.setValue(value); 
  function = calculator->getFunction();
  value.string = &function;
  functionProperty.setValue(value);
  adapter->update();
  
  delay(300);
}