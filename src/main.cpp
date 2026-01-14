#include "driver.h"
#include "battery.h"
#include "aqi.h"
#include "screen_ui1.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include "secrets.h"
#include <time.h>

// Sleep time in seconds
// #define SLEEP_TIME 1800 // 30 minutes
#define SLEEP_TIME 300           // 5 minutes
#define FULL_REFRESH_INTERVAL 48 // Full refresh every 48 wake-ups (24 hours)

// RTC memory persists through deep sleep
// RTC_DATA_ATTR int wakeCount = 0;
RTC_DATA_ATTR int wifiSelected = 0;
RTC_DATA_ATTR int fetchAqiHour = -1;
RTC_DATA_ATTR int aqi = -1;
RTC_DATA_ATTR int retryConnectedWifiCount = 0;

EPaper epaper;
#define ROTATION 2  // 0-3 rotation values
#define TEXT_FONT 1 // 1-8
#define NUM_TEXT_FONTS 7

const int BUTTON_KEY0 = 2; // KEY0 - GPIO2
const int BUTTON_KEY1 = 3; // KEY1 - GPIO3
const int BUTTON_KEY2 = 5; // KEY2 - GPIO5

// bool lastKey0State = HIGH;
// bool lastKey1State = HIGH;
// bool lastKey2State = HIGH;

const long gmtOffset = 25200; // GMT+7 7 * 3600 seconds
const int daylightOffset = 0;

void setup()
{
  Serial.begin(115200);
  delay(500);

  // Increment wake counter
  // wakeCount++;
  // Serial.printf("Wake count: %d\n", wakeCount);

  // Determine if full refresh is needed
  // bool needFullRefresh = (wakeCount >= FULL_REFRESH_INTERVAL);
  // if (needFullRefresh)
  // {
  //   Serial.println("Full refresh scheduled");
  //   wakeCount = 0; // Reset counter
  // }
  // else
  // {
  //   Serial.println("Partial refresh scheduled");
  // }

  analogReadResolution(12); // 12-bit ADC resolution
  pinMode(BATTERY_ADC, INPUT);
  pinMode(ADC_EN, OUTPUT);

  pinMode(BUTTON_KEY0, INPUT_PULLUP);
  pinMode(BUTTON_KEY1, INPUT_PULLUP);
  pinMode(BUTTON_KEY2, INPUT_PULLUP);

  if (retryConnectedWifiCount >= 3)
  {
    // After 3 failed attempts, switch to next WiFi option
    wifiSelected = (wifiSelected + 1) % 3;
    retryConnectedWifiCount = 0;
  }

  // Check button 1 state to determine which WiFi to use
  delay(100); // Allow time for button state to stabilize
  bool key0State = digitalRead(BUTTON_KEY0);
  bool key1State = digitalRead(BUTTON_KEY1);
  bool key2State = digitalRead(BUTTON_KEY2);
  wifiSelected = (key2State == LOW ? 2 : wifiSelected);
  wifiSelected = (key1State == LOW ? 1 : wifiSelected);
  wifiSelected = (key0State == LOW ? 0 : wifiSelected);

  switch (wifiSelected)
  {
  case 0:
    Serial.println("Connecting to Home WiFi...");
    // Regular WPA2/WPA3 connection (Home WiFi)
    WiFi.begin(ssid1, password1);
    break;
  case 1:
    Serial.println("Button 1 pressed - Connecting to Enterprise WiFi (WPA2)...");
    // WPA2-Enterprise connection (University WiFi)
    WiFi.begin(ssid2, WPA2_AUTH_PEAP, user, user, password2);
    break;
  case 2:
    Serial.println("Button 2 pressed - Connecting to Hotspot...");
    // Regular WPA2/WPA3 connection (Hotspot)
    WiFi.begin(ssid3, password3);
    break;
  default:
    Serial.println("Connecting to Home WiFi...");
    // Regular WPA2/WPA3 connection (Home WiFi)
    WiFi.begin(ssid1, password1);
    break;
  }

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20)
  {
    delay(500);
    Serial.print(".");
    attempt++;
  }

  epaper.begin();
  epaper.rotation = ROTATION; // Set rotation if needed
  // Serial.printf("Width: %d, Height: %d\n", epaper.width(), epaper.height());

  // Clear screen (turns white)
  epaper.fillScreen(TFT_WHITE);

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("WiFi connected.");
    retryConnectedWifiCount = 0;

    ui1();
  }
  else
  {
    Serial.println("WiFi connection failed.");
    epaper.textsize = 5;
    epaper.drawCentreString("WiFi connection failed.", 390, 160, TEXT_FONT);
    String wifiMethod;
    switch (wifiSelected)
    {
    case 0:
      wifiMethod = "Home WiFi";
      break;
    case 1:
      wifiMethod = "Enterprise WiFi";
      break;
    case 2:
      wifiMethod = "Hotspot";
      break;
    default:
      wifiMethod = "Home WiFi";
      break;
    }
    epaper.textsize = 2;
    epaper.drawCentreString("Method: " + wifiMethod, 380, 220, TEXT_FONT);
    retryConnectedWifiCount++;
    epaper.update();
  }

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  Serial.println("Entering deep sleep...");
  esp_sleep_enable_timer_wakeup(SLEEP_TIME * 1000000ULL);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_KEY0, 0); // Wake up on KEY0 (GPIO2) low
  esp_deep_sleep_start();
}

void loop() {}