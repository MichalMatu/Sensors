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

// Initialize the encoder
ESP32Encoder encoder;

int TVOC_SET = 0;
int eCO2_SET = 0;

void setup()
{
  Serial.begin(115200);

  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  pinMode(SW_PIN, INPUT_PULLUP);

  encoder.attachHalfQuad(CLK_PIN, DT_PIN);

  // Set the initial position of the encoder
  encoder.setCount(0);
  // initialize display and sgp30 sensor and encoder
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  sgp.begin();
}

void loop()
{
  long newPosition = encoder.getCount();

  // if (digitalRead(SW_PIN) == LOW)
  // {
  //   encoder.setCount(0); // Reset the encoder position
  // }

  // if enw position is greater than 8 or lower than -2 set to 0
  if (newPosition > 4 || newPosition < -2)
  {
    encoder.setCount(0);
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

  switch (newPosition / 2)
  {
  case -1:
    // display black screen to save power
    display.clearDisplay();
    display.display();
    break;

  case 0:
    if (currentMillis - lastDisplayUpdate >= displayUpdateInterval)
    {
      // Clear the display
      display.clearDisplay();

      // Set the text size to 1
      display.setTextSize(1);

      // Set the text color to white
      display.setTextColor(WHITE);

      // Display SGP30 measurements
      display.setCursor(0, 20);
      display.println("SGP30 TVOC: ");
      display.setCursor(75, 20);
      display.println(TVOC);
      display.setCursor(0, 30);
      display.println("SGP30 eCO2: ");
      display.setCursor(75, 30);
      display.println(eCO2);
      // convert currentmilis to second and display it
      display.setCursor(0, 50);
      display.println("RUN FOR: ");
      display.setCursor(70, 50);
      display.println(currentMillis / 1000);
      // Update the display
      display.display();

      lastDisplayUpdate = currentMillis;
    }
    break;

  case 1:
    if (currentMillis - lastDisplayUpdate >= displayUpdateInterval)
    {
      // display sgp30 readings on whole screen
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 10);
      display.println("SGP30:");
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
  case 2:
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
      display.setCursor(60, 30);
      display.println(TVOC_SET);
      // set cursor in new line
      display.setCursor(0, 50);
      display.println("eCO2: ");
      display.setCursor(60, 50);
      display.println(eCO2_SET);
      display.display();
      lastDisplayUpdate = currentMillis;
    }

    break;
  }
}
