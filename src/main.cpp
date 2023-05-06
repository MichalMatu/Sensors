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

void setup()
{
  Serial.begin(115200);

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

  unsigned long currentMillis = millis();
  // reset currentMillis to 0 after 30 days
  if (currentMillis > 2592000000)
  {
    currentMillis = 0;
  }


  int gassensorAnalog = analogRead(Gas_analog);
  int gassensorDigital = digitalRead(Gas_digital);

  if (!sgp.IAQmeasure())
  {
    Serial.println("Błąd podczas pomiaru IAQ!");
    return;
  }

  int TVOC = sgp.TVOC;
  int eCO2 = sgp.eCO2;

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
    display.setCursor(0, 30);
    display.println("SGP30 TVOC: ");
    display.setCursor(75, 30);
    display.println(TVOC);
    display.setCursor(0, 40);
    display.println("SGP30 eCO2: ");
    display.setCursor(75, 40);
    display.println(eCO2);

    // Update the display
    display.display();

    lastDisplayUpdate = currentMillis;
  }
}
