/* 
 * Adafruit Temperature and Humidity Sensor DHT22 
 */
#include "DHT.h"

#define DHTPIN 4       // DHT Pin
#define DHTTYPE DHT11  // DHT Type, DHT22 in this project

DHT dht(DHTPIN, DHTTYPE);

/* 
 * Adafruit Monochrome 0.96" 128x64 I2C OLED Display 
 */
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library
// On an esp32S:    D21(SDA), D22(SCL)
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  // < See datasheet for Address;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float tempDisplay = 99.0;   // Float to store initial temperature value for display
int humidDisplay = 99;      // Integer to store initial humidity value for display

/*  Function:       displaySetup
 *  Input:          None
 *  Output:         None
 *  Description:    Setup the SSD1306 OLED display according to example from Adafruit
 *                  Modified 2022/12/14 from Adafruit Example Code SSD1306 I2C
 */
void displaySetup(void) {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Error: SSD1306 allocation failed!"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen
  display.display();
}


/*  Function:       sensorSetup
 *  Input:          None
 *  Output:         None
 *  Description:    Setup the Temperature and Humidity Sensor DHT22 according to example from Adafruit
 *                  Modified 2022/12/14 from Adafruit Example Code DHT Sensor Library
 */
void sensorSetup(void) {
  dht.begin();
}

/*  Function:       drawLine
 *  Input:          char[]
 *  Output:         None
 *  Description:    Prints a line of characters to 0.91" OLED Display
 *                  Modified 2022/12/14 from Adafruit Example Code SSD1306 I2C
 */
void drawLine() {
  char myLine[5] = { ' ' };  // String to store temperature and humidity data for drawing to display
  display.clearDisplay();
  display.setTextSize(1);  // Normal 1:1 pixel scale
  display.setTextColor(WHITE);
  display.setFont(&FreeSans12pt7b);
  display.setCursor(0, 40);  // Start at top-left
  sprintf(myLine, "%5.1f", tempDisplay);
  display.print(myLine);
  display.print("C");
  display.setCursor(68, 40);  // Start at center-left
  sprintf(myLine, "%3i%%", humidDisplay);
  display.print(myLine);
  display.display();
}

/*  Function:       readTemperature
 *  Input:          None
 *  Output:         Temperature in float
 *  Description:    Returns the current temperature in celcius from DHT22 sensor and rounds the value for OLED printing
 */
float readTemperature(void) {
  float temperature = dht.readTemperature();

  if (isnan(temperature)) {
    Serial.println(F("Error: Failed to read Temperature!"));
    temperature = 99;
  }
  tempDisplay = temperature;
  return temperature;
}

/*  Function:       readHumidity
 *  Input:          None
 *  Output:         Humidity in float
 *  Description:    Returns the current humidity in percentage from DHT22 sensor and rounds the value for OLED printing
 */
float readHumidity(void) {
  float humidity = dht.readHumidity();

  if (isnan(humidity)) {
    Serial.println(F("Error: Failed to read Humidity!"));
    humidity = 99;
  }
  humidDisplay = round(humidity);
  return humidity;
}

/*  Struct:         DEV_TempSensor
 *  Description:    Service route for DHT22 temperature readings, DHT22 and SSD1306 OLED display must be initialized
 *                  first before executing
 *                  Modified 2022/12/14 from HomeSpan example 12: Service Loops
 *                  https://github.com/HomeSpan/HomeSpan/tree/master/examples/12-ServiceLoops
 */
struct DEV_TempSensor : Service::TemperatureSensor {
  SpanCharacteristic *temp;

  DEV_TempSensor() : Service::TemperatureSensor() {
    float temperature = readTemperature();
    temp = new Characteristic::CurrentTemperature(temperature);
    temp->setRange(-40, 80);
    drawLine();
    LOG1("Temperature Sensor Initialization Completed!");
    LOG1("\n");
  }

  void loop() {
    if (temp->timeVal() > 5000) {
      float temperature = readTemperature();
      temp->setVal(temperature);
      drawLine();
      LOG1("Temperature Update: ");
      LOG1(temperature);
      LOG1("\n");
    }
  }
};


/*  Struct:         DEV_HumiditySensor
 *  Description:    Service route for DHT22 humidity readings, DHT22 and SSD1306 OLED display must be initialized
 *                  first before executing
 *                  Modified 2022/12/14 from HomeSpan example 12: Service Loops
 *                  https://github.com/HomeSpan/HomeSpan/tree/master/examples/12-ServiceLoops
 */
struct DEV_HumiditySensor : Service::HumiditySensor {
  SpanCharacteristic *relativeHumidity;

  DEV_HumiditySensor() : Service::HumiditySensor() {
    float humidity = readHumidity();
    relativeHumidity = new Characteristic::CurrentRelativeHumidity(humidity);
    drawLine();
    LOG1("Humidity Sensor Initialization Completed!");
    LOG1("\n");
  }

  void loop() {
    if (relativeHumidity->timeVal() > 5000) {
      float humidity = readHumidity();
      relativeHumidity->setVal(humidity);
      drawLine();
      LOG1("Humidity Update: ");
      LOG1(humidity);
      LOG1("\n");    }
  }
};