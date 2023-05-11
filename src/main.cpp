#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_SGP30.h"
#include <ESP32Encoder.h>

#include <Preferences.h>
Preferences preferences;

TaskHandle_t buzzerTask = NULL; // buzzer task handle

void buzzer_task(void *parameter);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define CLK_PIN 26       // Define the encoder pins
#define DT_PIN 27        // Define the encoder pins
#define SW_PIN 25        // Define the encoder pins

const int buzzerPin = 18; // set the pin for the buzzer
const int RELAY_PIN = 5;  // set the pin for the relay

// declare if relar or buzzer will be in use, set true by default
bool buzzer = true;
bool relay = true;

unsigned long lastDisplayUpdate = 0;
unsigned long relay_update = 0;
unsigned long set_time = 0;
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

  // create task for buzzer melody
  xTaskCreatePinnedToCore(buzzer_task, "buzzer_task", 4096, NULL, 1, &buzzerTask, 1); // create task on core 1

  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  // set buzzer pin to low and relay pin to high by default
  digitalWrite(buzzerPin, LOW);
  digitalWrite(RELAY_PIN, HIGH);

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
  // check what core is being used
  Serial.print("loop() running on core ");
  Serial.println(xPortGetCoreID());
  unsigned long currentMillis = millis();

  newPosition = encoder.getCount() / 2;
  delta = newPosition - lastPosition;
  lastPosition = newPosition;

  if (digitalRead(SW_PIN) == LOW)
  {
    menu++;
    delay(200);
  }

  sgp.IAQmeasure();
  int TVOC = sgp.TVOC;
  int eCO2 = sgp.eCO2;
  if (currentMillis - lastDisplayUpdate >= displayUpdateInterval)
  {
    display.display();
    lastDisplayUpdate = currentMillis;
  }

  // map TVOC 0 - 400 to be in range from 10 to 50
  int TVOC_graph = map(TVOC, 0, 400, 5, 50);
  // map eCO2 400 - 2000 to be in range from 10 to 50
  int eCO2_graph = map(eCO2, 400, 4000, 5, 50);
  // map tvoc_set to draw horizontal line on screen
  int TVOC_set_graph = map(TVOC_SET, 0, 400, 5, 50);
  // map eCO2_set to draw horizontal line on screen
  int eCO2_set_graph = map(eCO2_SET, 400, 4000, 5, 50);

  switch (menu)
  {
  case 0:

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
    break;
  case 1:
    TVOC_SET += delta;
    // display set up alarm point:
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
    break;
  case 2:
    eCO2_SET += delta * 10;
    // display set up alarm point:
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
    break;
  case 3:
    // display simply graph with TVOC
    display.clearDisplay();
    // display TVOC
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(20, 0);
    display.println("TVOC");
    // display eCO2
    display.setCursor(80, 0);
    display.println("eCO2");
    // display TVOC_graph
    display.fillRect(10, 63 - TVOC_graph, 45, TVOC_graph, WHITE);
    // display eCO2_graph
    display.fillRect(70, 63 - eCO2_graph, 48, eCO2_graph, WHITE);

    // draw horizontal line on screen on first half of screen with tvoc_set_graph
    display.drawLine(10, 63 - TVOC_set_graph, 54, 63 - TVOC_set_graph, WHITE);
    // draw horizontal line on screen on second half of screen with eCO2_set_graph
    display.drawLine(70, 63 - eCO2_set_graph, 117, 63 - eCO2_set_graph, WHITE);
    break;
  case 4:
    // if delta is less than 0 add 60 to set_time if it's bigger than 0 add 1 to set_time
    if (delta < 0)
    {
      set_time += 60;
    }
    else if (delta > 0)
    {
      set_time += 1;
    }

    // declare hours and minutes variables
    int day;
    int hours;
    int minutes;
    // calculate hours and minutes, and limit their values to 24 and 60, respectively
    hours = (currentMillis / 60000 + set_time) / 60 % 24;
    minutes = (currentMillis / 60000 + set_time) % 60;
    day = (currentMillis / 60000 + set_time) / 1440;
    // display time on screen
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("TIME:");
    display.setCursor(0, 20);
    display.println("DAY:");
    display.setCursor(30, 20);
    display.println(day);
    display.setTextSize(3);
    display.setCursor(20, 40);
    if (hours < 10)
    {
      display.print("0");
    }
    display.print(hours);
    display.print(":");
    if (minutes < 10)
    {
      display.print("0");
    }
    display.print(minutes);
    break;
  case 5:
    if (delta < 0)
    {
      buzzer = buzzer ? false : true;
    }
    else if (delta > 0)
    {
      relay = relay ? false : true;
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("BUZZER:");
    display.setCursor(50, 25);
    display.setTextSize(2);
    if (buzzer)
    {
      display.println("ON");
    }
    else
    {
      display.println("OFF");
    }
    display.setCursor(0, 40);
    display.setTextSize(1);
    display.println("RELAY:");
    display.setCursor(50, 50);
    display.setTextSize(2);
    if (relay)
    {
      display.println("ON");
    }
    else
    {
      display.println("OFF");
    }
    break;

  case 6:
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

  if (eCO2 > eCO2_SET || TVOC > TVOC_SET)
  {
    if (relay)
    {
      digitalWrite(RELAY_PIN, LOW);
    }
    if (buzzer)
    {
      xTaskNotify(buzzerTask, 0, eNoAction);
    }
    relay_update = currentMillis;
  }
  else if (currentMillis - relay_update >= relay_time)
  {
    digitalWrite(RELAY_PIN, HIGH);
    noTone(buzzerPin);
  }
}

void buzzer_task(void *pvParameters)
{
  while (1)
  {
    // wait for a signal to play the melody
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    const int melody[] = {659, 659, 659, 523, 659, 784, 392, 523, 392, 330, 440, 494, 466, 440, 392, 659, 784, 880, 698, 784, 659, 523, 587, 494, 523, 392, 330, 440, 494, 466, 440, 392};

    const int noteDuration[] = {250, 250, 250, 250, 250, 250, 500, 250, 750, 250, 250, 250, 250, 250, 500, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 750, 250, 250, 250, 250, 500};

    for (int i = 0; i < sizeof(melody) / sizeof(int); i++)
    {
      tone(buzzerPin, melody[i], noteDuration[i]);
      delay(noteDuration[i] * 1.30);
    }
    noTone(buzzerPin);
  }
}

