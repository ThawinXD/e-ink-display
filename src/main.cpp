#include "driver.h"
#include "battery.h"
#include "aqi.h"
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
RTC_DATA_ATTR int wakeCount = 0;
RTC_DATA_ATTR int wifiSelected = 0;
RTC_DATA_ATTR int fetchAqiHour = -1;
RTC_DATA_ATTR int aqi = -1;

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
  wakeCount++;
  Serial.printf("Wake count: %d\n", wakeCount);

  // Determine if full refresh is needed
  bool needFullRefresh = (wakeCount >= FULL_REFRESH_INTERVAL);
  if (needFullRefresh)
  {
    Serial.println("Full refresh scheduled");
    wakeCount = 0; // Reset counter
  }
  else
  {
    Serial.println("Partial refresh scheduled");
  }

  analogReadResolution(12); // 12-bit ADC resolution
  pinMode(BATTERY_ADC, INPUT);
  pinMode(ADC_EN, OUTPUT);

  pinMode(BUTTON_KEY0, INPUT_PULLUP);
  pinMode(BUTTON_KEY1, INPUT_PULLUP);
  pinMode(BUTTON_KEY2, INPUT_PULLUP);

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

    configTime(gmtOffset, daylightOffset, "pool.ntp.org", "time.navy.mi.th");
    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
      Serial.println(&timeinfo, "Current time: %Y-%m-%d %H:%M:%S");

      char dateStr[15];
      strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);

      char dayStr[10];
      strftime(dayStr, sizeof(dayStr), "%A", &timeinfo);

      char timeStr[10];
      strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);

      int hour = timeinfo.tm_hour;
      if (fetchAqiHour != hour || aqi == -1)
      {
        aqi = fetchAqiDataFromIqair();
        fetchAqiHour = hour;
      }

      epaper.textsize = 1;
      epaper.textfont = TEXT_FONT;

      epaper.drawString(dateStr, 10, 40, NUM_TEXT_FONTS);

      epaper.textsize = 5;
      epaper.drawRightString(dayStr, 790, 40, TEXT_FONT);

      epaper.textsize = 3;
      epaper.drawCentreString(timeStr, 380, 160, NUM_TEXT_FONTS);

      if (aqi != -1)
      {
        char aqiStr[20];
        sprintf(aqiStr, "AQI: %d", aqi);
        epaper.textsize = 3;
        epaper.drawString(aqiStr, 10, 400, TEXT_FONT);

        String aqiLabel;
        if (aqi <= 12)
        {
          aqiLabel = "Good";
        }
        else if (aqi <= 36)
        {
          aqiLabel = "Moderate";
        }
        else if (aqi <= 56)
        {
          aqiLabel = "Unhealthy for Sensitive Groups";
        }
        else if (aqi <= 151)
        {
          aqiLabel = "Unhealthy";
        }
        else if (aqi <= 251)
        {
          aqiLabel = "Very Unhealthy";
        }
        else
        {
          aqiLabel = "Hazardous";
        }
        epaper.drawString(aqiLabel, 10, 440, TEXT_FONT);
      }
      else
      {
        epaper.textsize = 2;
        epaper.drawString("AQI data unavailable", 10, 440, TEXT_FONT);
        // wifiSelected = (wifiSelected + 1) % 3;
      }

      char batteryStr[30];
      epaper.textsize = 3;
      sprintf(batteryStr, "Battery: %.2f V", readBatteryVoltage());
      epaper.drawRightString(batteryStr, 790, 430, TEXT_FONT);
    }
  }
  else
  {
    Serial.println("WiFi connection failed.");
    epaper.textsize = 5;
    epaper.drawCentreString("WiFi connection failed.", 390, 160, TEXT_FONT);
    epaper.textsize = 2;
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
    epaper.drawCentreString("Method: " + wifiMethod, 380, 220, TEXT_FONT);
  }

  epaper.update();

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  Serial.println("Entering deep sleep...");
  esp_sleep_enable_timer_wakeup(SLEEP_TIME * 1000000ULL);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_KEY0, 0); // Wake up on KEY0 (GPIO2) low
  esp_deep_sleep_start();
}

void loop()
{
  // float batteryVoltage = readBatteryVoltage();
  // bool key0State = digitalRead(BUTTON_KEY0);
  // bool key1State = digitalRead(BUTTON_KEY1);
  // bool key2State = digitalRead(BUTTON_KEY2);

  // if (key0State != lastKey0State) {
  //   Serial.printf("KEY0 is %s\n", key0State == LOW ? "Pressed" : "Released");
  //   lastKey0State = key0State;
  //   delay(50); // Debounce delay
  // }
  // if (key1State != lastKey1State) {
  //   Serial.printf("KEY1 is %s\n", key1State == LOW ? "Pressed" : "Released");
  //   lastKey1State = key1State;
  //   delay(50); // Debounce delay
  // }
  // if (key2State != lastKey2State) {
  //   Serial.printf("KEY2 is %s\n", key2State == LOW ? "Pressed" : "Released");
  //   lastKey2State = key2State;
  //   delay(50); // Debounce delay
  // }

  // Serial.printf("Battery Voltage: %.2f V\n", batteryVoltage);
  // Serial.println("Test loop.");
  // delay(1000);
  // Serial.println("Test loop..");
  // delay(1000);
  // Serial.println("Test loop...");
  // delay(1000);

  // delay(10);
}