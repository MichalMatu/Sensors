#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_SGP30.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

unsigned long lastDisplayUpdate = 0;
const unsigned long displayUpdateInterval = 100;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int Gas_analog = 4;  // used for ESP32
int Gas_digital = 2; // used for ESP32

Adafruit_SGP30 sgp;

// Include the ESP32 encoder library
#include <ESP32Encoder.h>

// Define the encoder pins
#define CLK_PIN 26
#define DT_PIN 27
#define SW_PIN 25

// Initialize the encoder
ESP32Encoder encoder;

void setup()
{
  Serial.begin(115200);

  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  pinMode(SW_PIN, INPUT_PULLUP);

  encoder.attachHalfQuad(CLK_PIN, DT_PIN);

  // Set the initial position of the encoder
  encoder.setCount(0);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(2000);

  pinMode(Gas_digital, INPUT);

  if (!sgp.begin())
  {
    Serial.println("Nie udało się uruchomić sensora SGP30. Sprawdź połączenie.");
    while (1)
      ;
  }
}

void loop()
{

  long newPosition = encoder.getCount();

  if (digitalRead(SW_PIN) == LOW)
  {
    encoder.setCount(0); // Reset the encoder position
  }

  unsigned long currentMillis = millis();
  // reset currentMillis to 0 after 30 days
  if (currentMillis > 2592000000)
  {
    currentMillis = 0;
  }

  int gassensorAnalog = analogRead(Gas_analog);
  int gassensorDigital = digitalRead(Gas_digital);

  sgp.IAQmeasure();
  int TVOC = sgp.TVOC;
  int eCO2 = sgp.eCO2;

  switch (newPosition / 2)
  {
  case 0:
    if (currentMillis - lastDisplayUpdate >= displayUpdateInterval)
    {
      // Clear the display
      display.clearDisplay();

      // Set the text size to 1
      display.setTextSize(1);

      // Set the text color to white
      display.setTextColor(WHITE);

      // Set the cursor position to (0, 10)
      display.setCursor(0, 10);
      display.println("MQ2 D: ");
      display.setCursor(35, 10);
      display.println(gassensorDigital);
      // set cursor in new line
      display.setCursor(45, 10);
      display.println("A: ");
      display.setCursor(60, 10);
      display.println(gassensorAnalog);

      // Display SGP30 measurements
      display.setCursor(0, 20);
      display.println("SGP30 TVOC: ");
      display.setCursor(75, 20);
      display.println(TVOC);
      display.setCursor(0, 30);
      display.println("SGP30 eCO2: ");
      display.setCursor(75, 30);
      display.println(eCO2);
      // set cursor to new line and display encoder newposition
      display.setCursor(0, 40);
      display.println("Encoder: ");
      display.setCursor(60, 40);
      display.println(newPosition / 2);
      // convert currentmilis to second and display it
      display.setCursor(0, 50);
      display.println("Millis: ");
      display.setCursor(60, 50);
      display.println(currentMillis / 1000);

      // Update the display
      display.display();

      lastDisplayUpdate = currentMillis;
    }
    break;
  case 1:
    if (currentMillis - lastDisplayUpdate >= displayUpdateInterval)
    {
      // display mq2 readings on whole screen
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 10);
      display.println("MQ2 Sensor");
      display.setCursor(0, 30);
      display.println("Dig: ");
      display.setCursor(60, 30);
      display.println(gassensorDigital);
      // set cursor in new line
      display.setCursor(0, 50);
      display.println("Ana: ");
      display.setCursor(60, 50);
      display.println(gassensorAnalog);
      display.display();
      lastDisplayUpdate = currentMillis;
    }
      break;
    case 2:
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

      default:
        // display whole white screen
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.println("Encoder:");
        display.setTextSize(3);
        display.setCursor(50, 40);
        display.println(newPosition / 2);
        display.display();
        break;
      }
}
