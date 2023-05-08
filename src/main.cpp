#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_SGP30.h"
#include <ESP32Encoder.h>

#include <Preferences.h>
Preferences preferences;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define CLK_PIN 26       // Define the encoder pins
#define DT_PIN 27        // Define the encoder pins
#define SW_PIN 25        // Define the encoder pins

const int RELAY_PIN = 5; // set the pin for the relay

unsigned long lastDisplayUpdate = 0;
unsigned long relay_update = 0;
const unsigned long displayUpdateInterval = 100;
const unsigned long relay_time = 5000;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Adafruit_SGP30 sgp;

// Initialize the encoder
ESP32Encoder encoder;

long lastPosition = 0;
long newPosition = 0;
int delta = 0;
int delta1 = 0;

int TVOC_SET = 50;
int eCO2_SET = 500;

int menu = 0;

void setup()
{
  Serial.begin(115200);

  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  pinMode(SW_PIN, INPUT_PULLUP);

  pinMode(RELAY_PIN, OUTPUT); // set the relay pin as an output

  encoder.attachHalfQuad(CLK_PIN, DT_PIN);

  // Set the initial position of the encoder
  encoder.setCount(0);
  // initialize display and sgp30 sensor and encoder
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  sgp.begin();
  preferences.begin("my_app", false);
  TVOC_SET = preferences.getInt("TVOC_SET", 50);
  eCO2_SET = preferences.getInt("eCO2_SET", 500);
}

void loop()
{
  newPosition = encoder.getCount();
  delta = newPosition - lastPosition;
  lastPosition = newPosition;
  if (digitalRead(SW_PIN) == LOW)
  {
    menu++;
    delay(200);
  }

  unsigned long currentMillis = millis();
  // reset currentMillis to 0 after 30 days
  if (currentMillis > 2592000000)
  {
    currentMillis = 0;
  }

  sgp.IAQmeasure();
  int TVOC = sgp.TVOC;
  int eCO2 = sgp.eCO2;

  switch (menu)
  {
  case 0:
    if (currentMillis - lastDisplayUpdate >= displayUpdateInterval)
    {
      // display sgp30 readings on whole screen
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 10);
      display.println("SGP30:");
      display.setTextSize(1);
      display.setCursor(100, 10);
      display.println(currentMillis / 60000);
      display.setTextSize(2);
      display.setCursor(0, 30);
      display.println("TVOC: ");
      display.setCursor(60, 30);
      display.println(TVOC);
      // set cursor in new line
      display.setCursor(0, 50);
      display.println("eCO2: ");
      display.setCursor(60, 50);
      display.println(eCO2);
      display.display();
      lastDisplayUpdate = currentMillis;
    }
    break;
  case 1:
    TVOC_SET += delta;
    // display set up alarm point:
    if (currentMillis - lastDisplayUpdate >= displayUpdateInterval)
    {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 10);
      display.println("SET UP ALARM POINT:");
      display.setCursor(0, 30);
      display.println("TVOC: ");
      display.setCursor(50, 40);
      display.setTextSize(3);
      display.println(TVOC_SET);
      display.display();
      lastDisplayUpdate = currentMillis;
    }
    break;
  case 2:
    eCO2_SET += delta * 10;
    // display set up alarm point:
    if (currentMillis - lastDisplayUpdate >= displayUpdateInterval)
    {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 10);
      display.println("SET UP ALARM POINT:");
      // set cursor in new line
      display.setCursor(0, 50);
      display.println("eCO2: ");
      display.setCursor(50, 40);
      display.setTextSize(3);
      display.println(eCO2_SET);
      display.display();
      lastDisplayUpdate = currentMillis;
    }
    break;
  case 3:
  // display simply graph with TVOC 
    if (currentMillis - lastDisplayUpdate >= displayUpdateInterval)
    {
      display.clearDisplay();
    // display dot in each corner
      display.drawPixel(0, 0, WHITE);
      display.drawPixel(0, 63, WHITE);
      display.drawPixel(127, 0, WHITE);
      display.drawPixel(127, 63, WHITE);
      // map eco2 to be between 0 and 63 now it can be between 400 and 2000
      int eco2 = map(eCO2, 400, 2000, 63, 0);
      // display lines using eco2 as hight on screen
      display.drawLine(0, 63, 0, eco2, WHITE);
      display.display();
      lastDisplayUpdate = currentMillis;
    }
  break;

  case 4:
    // display black screen to save power
    display.clearDisplay();
    display.display();
    break;
  default:
    menu = 0;
    break;
  }

  if (delta != 0 || delta1 != 0)
  {
    preferences.putInt("TVOC_SET", TVOC_SET);
    preferences.putInt("eCO2_SET", eCO2_SET);
  }

  if ((eCO2 > eCO2_SET || TVOC > TVOC_SET) && (currentMillis - relay_update >= relay_time))
  {
    digitalWrite(RELAY_PIN, HIGH);
    relay_update = currentMillis;
  }
  else if (currentMillis - relay_update < relay_time)
  {
    digitalWrite(RELAY_PIN, HIGH);
  }
  else
  {
    digitalWrite(RELAY_PIN, LOW);
  }
}
