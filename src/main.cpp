#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_SGP30.h"
#include <ESP32Encoder.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Preferences.h>

// Default network credentials
char ssid[32] = "ESP32AP";
char password[64] = "0123456789";

void buzzer_task(void *parameter);

const int SCREEN_WIDTH = 128; // OLED display width, in pixels
const int SCREEN_HEIGHT = 64; // OLED display height, in pixels
const int CLK_PIN = 26;       // CLK pin of the rotary encoder
const int DT_PIN = 27;        // DT pin of the rotary encoder
const int SW_PIN = 25;        // SW pin of the rotary encoder
const int buzzerPin = 18;     // buzzer pin
const int RELAY_PIN = 5;      // relay pin

// define buffer size for average sensor readings buffer for tvoc and buffer1 for eCO2
const int BUFFER_SIZE = 60;
int buffer[BUFFER_SIZE];
int buffer1[BUFFER_SIZE];
int bufferIndex = 0;
int sum;
int sum1;
int average;
int average1;

unsigned long lastDisplayUpdate = 0;
unsigned long relay_update = 0;
unsigned long set_time = 0;
unsigned long lastReadingTime = 0;
int displayUpdateInterval = 200;
int relay_time = 5000;
int readingInterval = 1000;

long lastPosition = 0;
long newPosition = 0;
int delta = 0;
int delta1 = 0;

int TVOC;
int eCO2;

int TVOC_SET;
int eCO2_SET;
bool buzzer;
bool relay;
unsigned long currentMillis = 0;

int menu = 0;
int menu_clock = 0;
int menu_set = 0;
bool menu_scroll = true;

// Create an instances
AsyncWebServer server(80);
Preferences preferences;
TaskHandle_t buzzerTask = NULL; // buzzer task handle
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_SGP30 sgp;
ESP32Encoder encoder;

#if !(USING_DEFAULT_ARDUINO_LOOP_STACK_SIZE)
uint16_t USER_CONFIG_ARDUINO_LOOP_STACK_SIZE = 8192;
#endif

void disable_watchdog()
{
  disableCore0WDT();
  disableCore1WDT();
}

void handleValuesRequest(AsyncWebServerRequest *request)
{
  String values = "TVOC: " + String(TVOC) + "<br>eCO2: " + String(eCO2);
  request->send(200, "text/html", values);
}

void handleSaveCredentialsRequest(AsyncWebServerRequest *request)
{
  String submittedSSID = request->arg("ssid");
  String submittedPassword = request->arg("password");

  // Check if the submitted SSID is not empty and the password has at least 8 characters
  if (submittedSSID.length() > 0 && submittedPassword.length() >= 8 && submittedPassword.length() <= 64 && submittedSSID.length() <= 32 && submittedPassword != ssid && submittedSSID != password)
  {
    // Update the credentials
    strncpy(ssid, submittedSSID.c_str(), sizeof(ssid));
    strncpy(password, submittedPassword.c_str(), sizeof(password));

    request->send(SPIFFS, "/valid.html", "text/html");
    // Disconnect any connected clients
    WiFi.softAPdisconnect();
    // Configure ESP32 as an access point with new credentials
    WiFi.softAP(ssid, password);
  }
  else
  {
    request->send(SPIFFS, "/invalid-credentials.html", "text/html");
  }
}

void setup()
{
  // Serial.begin(115200);

  WiFi.softAP(ssid, password);

  // Get IP address of the access point
  IPAddress ip = WiFi.softAPIP();

  SPIFFS.begin(true);

  // Serve static files from SPIFFS
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  // favicons < -- work on this
  // server.serveStatic("/favicon.ico", SPIFFS, "/favicon.ico");

  server.on("/values", HTTP_GET, handleValuesRequest);
  server.on("/save-credentials", HTTP_POST, handleSaveCredentialsRequest);

  // Start the server
  server.begin();

  // create task for buzzer melody
  xTaskCreatePinnedToCore(buzzer_task, "buzzer_task", 4096, NULL, 1, &buzzerTask, 1); // create task on core 1

  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  pinMode(SW_PIN, INPUT_PULLUP);

  // set buzzer pin to low and relay pin to high by default
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  // set the relay pin as an output
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  encoder.attachHalfQuad(CLK_PIN, DT_PIN);

  // Set the initial position of the encoder
  encoder.setCount(0);
  // initialize display and sgp30 sensor and encoder
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  sgp.begin();
  preferences.begin("my_app", false);
  // declare ane retrive tvoc_set and eco2_set from memory if no value set to default
  TVOC_SET = preferences.getInt("TVOC_SET", 50);
  eCO2_SET = preferences.getInt("eCO2_SET", 500);
  // declare if relay or buzzer will be in use, set true by default
  buzzer = preferences.getBool("buzzer", true);
  relay = preferences.getBool("relay", true);
}

void loop()
{
  currentMillis = millis();

  newPosition = encoder.getCount() / 2;
  delta = newPosition - lastPosition;
  lastPosition = newPosition;

  if (menu_scroll)
  {
    if (delta > 0)
    {
      menu++;
    }
    else if (delta < 0)
    {
      menu--;
    }
  }
  if (currentMillis - lastReadingTime >= readingInterval)
  {
    // take sensor readings
    sgp.IAQmeasure();
    TVOC = sgp.TVOC;
    eCO2 = sgp.eCO2;

    if (currentMillis > 20000)
    {
      // Update buffer with new value
      buffer[bufferIndex] = TVOC;
      buffer1[bufferIndex] = eCO2;
      // Move to the next buffer index
      bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;

      // Calculate average of the last 60 values
      sum = 0;
      sum1 = 0;
      for (int i = 0; i < BUFFER_SIZE; i++)
      {
        sum += buffer[i];
        sum1 += buffer1[i];
      }
      average = sum / BUFFER_SIZE;
      average1 = sum1 / BUFFER_SIZE;
    }
    lastReadingTime = currentMillis;
  }

  if (currentMillis - lastDisplayUpdate >= displayUpdateInterval)
  {
    display.display();
    lastDisplayUpdate = currentMillis;
  }

  switch (menu)
  {
  case -1:
    // display WI-FI settings:
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("WiFi Settings:");
    display.setCursor(0, 10);
    // display ssid
    display.print("SSID: ");
    display.setCursor(30, 10);
    display.print(ssid);
    display.setCursor(0, 20);
    // display password
    display.print("Pass: ");
    display.setCursor(30, 20);
    display.print(password);
    // display ip address
    display.setCursor(0, 30);
    display.print("IP: ");
    display.setCursor(20, 30);
    display.print(WiFi.softAPIP());
    // display mac address
    display.setCursor(0, 40);
    display.print(WiFi.softAPmacAddress());
    // display wifi status
    display.setCursor(0, 50);
    display.print("Status: ");
    display.setCursor(45, 50);
    display.print(WiFi.status());
    break;
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
    // if tvoc is bigger than tvocset display dot
    if (TVOC > TVOC_SET)
    {
      display.setCursor(110, 30);
      display.print("*");
    }

    // set cursor in new line
    display.setCursor(0, 50);
    display.println("eCO2: ");
    display.setCursor(60, 50);
    display.println(eCO2);
    // if eco2 is bigger than eco2set display dot
    if (eCO2 > eCO2_SET)
    {
      display.setCursor(110, 50);
      display.print("*");
    }
    break;
  case 1:
    if (digitalRead(SW_PIN) == LOW)
    {
      menu_scroll = menu_scroll ? false : true;
      delay(200);
    }

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
    if (!menu_scroll)
    {
      TVOC_SET += delta;
      display.drawLine(44, 63, 110, 63, WHITE);
    }

    break;
  case 2:
    if (digitalRead(SW_PIN) == LOW)
    {
      menu_scroll = menu_scroll ? false : true;
      delay(200);
    }

    // display set up alarm point:
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("SET UP ALARM POINT:");
    // set cursor in new line
    display.setCursor(0, 50);
    display.println("eCO2: ");
    display.setCursor(45, 40);
    display.setTextSize(3);
    display.println(eCO2_SET);
    if (!menu_scroll)
    {
      eCO2_SET += delta * 10;
      display.drawLine(40, 63, 120, 63, WHITE);
    }
    break;

  case 3:
    if (digitalRead(SW_PIN) == LOW)
    {
      menu_scroll = menu_scroll ? false : true;
      menu_clock++;
      delay(200);
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
    display.setCursor(0, 0);
    display.println("CLOCK:");
    display.setCursor(60, 0);
    display.println("DAY:");
    display.setCursor(90, 0);
    display.println(day);
    display.setTextSize(3);
    display.setCursor(20, 35);
    if (hours < 10)
    {
      display.print("0");
    }

    display.print(hours);
    if (menu_clock == 1)
    {
      display.drawLine(20, 63, 55, 63, WHITE);
      if (delta > 0)
      {
        set_time += 60;
      }
      else if (delta < 0 && set_time >= 60)
      {
        set_time -= 60;
      }
    }
    display.print(":");
    if (minutes < 10)
    {
      display.print("0");
    }

    display.print(minutes);
    if (menu_clock == 2)
    {
      menu_scroll = false;
      display.drawLine(75, 63, 110, 63, WHITE);
      if (delta > 0)
      {
        set_time += 1;
      }
      else if (delta < 0 && set_time >= 1)
      {
        set_time -= 1;
      }
    }
    if (menu_clock > 2)
    {
      menu_clock = 0;
    }
    break;
  case 4:
    if (digitalRead(SW_PIN) == LOW)
    {
      menu_scroll = menu_scroll ? false : true;
      menu_set++;
      delay(200);
    }
    if (menu_set == 1)
    {
      menu_scroll = false;

      if (delta < 0 || delta > 0)
      {
        buzzer = buzzer ? false : true;
        preferences.putBool("buzzer", buzzer);
      }
    }

    if (menu_set == 2)
    {
      menu_scroll = false;

      if (delta > 0 || delta < 0)
      {
        relay = relay ? false : true;
        preferences.putBool("relay", relay);
      }
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("BUZZER:");
    display.setCursor(60, 15);
    display.setTextSize(2);
    if (buzzer)
    {
      display.println("ON");
    }
    else
    {
      display.println("OFF");
    }
    if (menu_set == 1)
    {
      display.drawLine(55, 35, 95, 35, WHITE);
    }
    display.setCursor(0, 30);
    display.setTextSize(1);
    display.println("RELAY:");
    display.setCursor(60, 40);
    display.setTextSize(2);
    if (relay)
    {
      display.println("ON");
    }
    else
    {
      display.println("OFF");
    }
    if (menu_set == 2)
    {
      display.drawLine(55, 60, 95, 60, WHITE);
    }
    if (menu_set > 2)
    {
      menu_set = 0;
      menu_scroll = true;
    }
    break;

  case 5:
    // display average an screen
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("AVERAGE TVOC 30s:");
    display.setTextSize(2);
    if (currentMillis > 60000)
    {
      display.setCursor(55, 15);
      display.println(average);
    }
    else
    {
      display.setCursor(35, 15);
      display.println("wait...");
    }
    display.setCursor(0, 35);
    display.setTextSize(1);
    display.println("AVERAGE eCO2 30s:");
    display.setTextSize(2);
    if (currentMillis > 60000)
    {
      display.setCursor(55, 50);
      display.println(average1);
    }
    else
    {
      display.setCursor(35, 50);
      display.println("wait...");
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

  // of buzzer or relay change value from true to false or from false to true

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
  }
}

void buzzer_task(void *pvParameters)
{
  while (1)
  {
    // wait for a signal to play the melody
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    const int melody[] = {// Mario Bros theme song melody
                          659, 659, 0, 659, 0, 523, 659, 0, 784, 0, 392, 0, 523, 0, 392, 0,
                          330, 0, 440, 0, 494, 0, 466, 0, 440, 0, 392, 0, 659, 0, 784, 0,
                          880, 0, 698, 0, 784, 0, 659, 0, 523, 0, 587, 0, 494, 0, 523, 0,
                          392, 0, 330, 0, 440, 0, 494, 0, 466, 0, 440, 0, 392, 0};

    const int noteDuration[] = {// Mario Bros theme song note duration
                                125, 125, 125, 125, 125, 125, 125, 125, 250, 125, 250, 125, 125, 125, 125, 125,
                                125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 250, 125, 250, 125,
                                125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125,
                                125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125};

    for (int i = 0; i < sizeof(melody) / sizeof(int); i++)
    {
      tone(buzzerPin, melody[i], noteDuration[i]);
      delay(noteDuration[i] * 1.30);
    }
    noTone(buzzerPin);
  }
}
