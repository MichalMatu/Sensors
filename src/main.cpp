#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_SGP30.h"
#include <ESP32Encoder.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

unsigned long lastDisplayUpdate = 0;
const unsigned long displayUpdateInterval = 100;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Adafruit_SGP30 sgp;

// Define the encoder pins
#define CLK_PIN 26
#define DT_PIN 27
#define SW_PIN 25

const int RELAY_PIN = 5; // set the pin for the relay

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
}

void loop()
{
  newPosition = encoder.getCount();

  if (digitalRead(SW_PIN) == LOW)
  {
    menu++;
    delay(200);
  }
  if (menu > 3)
  {
    menu = 0;
  }

  // if menu is 0 swich off encoder.getcount
  if (menu == 0)
  {
    encoder.setCount(0);
  }

  if (menu == 1)
  {
    delta = newPosition - lastPosition;
    if (delta != 0)
    {
      TVOC_SET += delta;
      // Update lastPosition to current position
      lastPosition = newPosition;
    }
  }

  if (menu == 2)
  {
    delta1 = newPosition - lastPosition;
    if (delta1 != 0)
    {
      eCO2_SET += delta1;
      // Update lastPosition to current position
      lastPosition = newPosition;
    }
  }
  Serial.println("eCO2_SET = " + String(eCO2_SET));
  Serial.println("TVOC_SET = " + String(TVOC_SET));

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
      display.setCursor(60, 40);
      display.setTextSize(3);
      display.println(TVOC_SET);
      display.display();
      lastDisplayUpdate = currentMillis;
    }
    break;
  case 2:
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
      display.setCursor(60, 40);
      display.setTextSize(3);
      display.println(eCO2_SET);
      display.display();
      lastDisplayUpdate = currentMillis;
    }
    break;

  case 3:
    // display black screen to save power
    display.clearDisplay();
    display.display();
    break;
  }
}
