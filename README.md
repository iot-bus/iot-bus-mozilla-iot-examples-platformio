# IoT-Bus mozilla IoT Examples

These examples are designed to run on IoT-Bus boards by oddWires. In each case, they demonstrate how to create a mozilla-iot "thing" and expose it through the mozilla-iot gateway running on a raspberry pi on the same Wi-Fi network. We will refer to a mozilla-iot thing as a Thing in this document. To setup a gateway see here: 

https://hacks.mozilla.org/2018/02/how-to-build-your-own-private-smart-home-with-a-raspberry-pi-and-mozillas-things-gateway/

In each case you will have to enter your ssid and password within the main cpp file otherwise you will not reach the gateway. 

    //TODO: Hardcode your wifi credentials here (and keep it private)
    const char* ssid = "........";
    const char* password = "........";

These examples are structured for Platformio but can be run using the Arduino IDE by renaming the source file as .ino and saving it in a folder of the same name. If you run under Arduino you'll have to ensure each library is installed in the "Arduino/Libraries" folder. You can find the library dependencies for each in the platformio.ini file.

![Iot-Bus-mozilla-iot-example](RelayOn.jpg)

## IoT-Bus-LED_Thing

This example create a Thing that enables the built-in LED on GPIO5 to be set through the mozilla-iot domain.

## IoT-Bus-LED_Thing

This example create a Thing that enables the status and brightness of the built-in LED on GPIO5 to be set through the mozilla-iot domain.

## IoT-Bus_DHT11_Thing

This example creates a Thing that exposes termperature and humidity properties and also displays the temperature locally.

Connect a DHT11 Temperature Sensor and run:
    
    DATA -> GPIO4
    VCC  -> VUSB (VIN is OK if using battery(3.7-42V) but not 3V3 as not high enough)
    GND  -> GND

## IoT-Bus_HC-SR04_Thing

This example creates a Thing that exposes the current distance reading or the HC-SR04 ultransonic distance sensor and also displays the distance locally.

Connect an HC-SR04 Ultrasonic Distance Sensor and run:

    TRIG -> GPIO2
    ECHO -> GPIO4
    VCC  -> VUSB
    GND  -> GND (VIN is OK if using battery(3.7-42V) but not 3V3 as not high enough)

## IoT-HC-SR501_PIR_Thing

This example creates a Motion Sensor Thing that triggers on movement. It sets the on-board LED on triggering and updates the mozilla-iot interface appropriately.

Connect an HC-SR501 Passive Infrared Sensor or any similar device and run:
    
    OUT  -> GPIO4 
    VCC  -> VUSB (Typically VIN is OK if using battery(3.7-42V) but not 3V3 as not high enough)    
    GND  -> GND 

## IoT-Touch_Thing

This example creates a Touch Switch Thing that triggers when one of the capacitive touch pins on the ESP32 are touched. It sets the on-board LED on triggering and updates the mozilla-iot interface appropriately.

Connect a wire to GPIO4 and run.   

## IoT-Bus_Calculator_Thing    

This example is a simple integer calculator that creates a Thing that exposes the two numbers, the last function and the result. It requires the IoT-Bus display.

## IoT-Bus_Window_Door_Sensor_Thing

This example shows how to use a typical magentic door sensor. Just connect one soide of the contacts to GPIO4 and the other to GND. When the contacts are open the door sensor will show open in the mozilla interface and when they are shut the door will show shut.

## IoT-Bus_Relay_Thing and IoT-Bus_Relay_Thing_Display_Touch

These examples uses an IoT-Bus relay board together with an IoT-Bus Io processor to expose the relay status and to enable the user to change through your mozilla-iot domain. In the second example the current status of the relay is also shown on the display. The touchscreen is enabled so it can be switched on and off locally. The status will be reflected by mozilla-iot.