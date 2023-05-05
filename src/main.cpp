/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
*********/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int Gas_analog = 4;  // used for ESP32
int Gas_digital = 2; // used for ESP32

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
}

void loop()
{

  int gassensorAnalog = analogRead(Gas_analog);
  int gassensorDigital = digitalRead(Gas_digital);

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

  // Update the display
  display.display();

  delay(100);
}
