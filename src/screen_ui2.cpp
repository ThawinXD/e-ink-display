#include "screen_ui2.h"
#include "battery.h"
#include "api.h"
#include <time.h>

#define TEXT_FONT 1
#define NUM_TEXT_FONTS 7

static RTC_DATA_ATTR int dateDay = -1;
static RTC_DATA_ATTR int fetchAqi = -1;
static RTC_DATA_ATTR int aqi = -1;
static RTC_DATA_ATTR int fetchWeather = -1;
static RTC_DATA_ATTR forecasthourly weatherForecast;
static RTC_DATA_ATTR char dateStr[15];
static RTC_DATA_ATTR char dayStr[10];

static void updateDateDisplay(char* dateStr)
{
  epaper.textsize = 1;
  epaper.drawString(dateStr, 10, 5, NUM_TEXT_FONTS);
};

static void updateDayDisplay(char* dayStr)
{
  epaper.textsize = 4;
  epaper.drawRightString(dayStr, 790, 20, TEXT_FONT);
};

static void updateAqiDisplay(const int &aqi)
{
  if (aqi != -1)
  {
    char aqiStr[20];
    sprintf(aqiStr, "AQI: %d", aqi);
    epaper.textsize = 2;
    epaper.drawString(aqiStr, 10, 420, TEXT_FONT);

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
  }
};

static void lastUpdateTime(const struct tm &timeinfo)
{
  char timeStr[20];
  strftime(timeStr, sizeof(timeStr), "Last update: %H:%M", &timeinfo);
  epaper.textsize = 2;
  epaper.drawCentreString(timeStr, 400, 440, TEXT_FONT);
};

static void updateBatteryDisplay()
{
  char batteryStr[30];
  epaper.textsize = 2;
  sprintf(batteryStr, "Battery: %.2f V", readBatteryVoltage());
  epaper.drawRightString(batteryStr, 790, 440, TEXT_FONT);
};

static void drawline() {
  epaper.drawLine(10, 240, 790, 240, TFT_BLACK);
  float indent = 133.33;
  for(int i = 1; i < 6; i++) {
    float x = floor(800 / 6.0 * i);
    epaper.drawLine(x, 80, x, 400, TFT_BLACK);
  }
};

static void updateWeatherDisplay(const forecasthourly &forecast, int currentHour)
{
  Serial.println("Updating weather display...");
  Serial.printf("Current hour: %d\n", currentHour);
  int idxHour = (currentHour >= 7 && currentHour <= 18)? 7: (currentHour >= 0 && currentHour < 7)? 0 : 19;

  // first range from this day 07 08 09 10 11 12 13 14 15 16 17 18
  // second range from this day 19 20 21 22 23 next day 00 01 02 03 04 05 06
  // or prev day 19 20 21 22 23 this day 00 01 02 03 04 05 06
  
  float indent = 133.33;
  int i = 0;
  for(int j = 0; j <= 11; j++, i++) {
    int shift = (j > 5) ? 168 : 0; // Shift ui
    if (j == 6) i = 0;
    if (forecast.hour[idxHour] == -1 || (currentHour >= 0 && currentHour < 7 && idxHour < 0)) {
      idxHour = (idxHour + 1) % 24; // Move to the next hour, wrap around at 24
      // Serial.println("HERE" + String(idxHour));
      continue; // Skip if data is unavailable
    }

    int hour = forecast.hour[idxHour];
    float tempC = forecast.tempC[idxHour];
    int humidity = forecast.humidity[idxHour];
    int rainChance = forecast.rainChance[idxHour];

    // char weatherStr[50];
    // sprintf(weatherStr, "%02d:00 - Temp: %.1f C, Humidity: %d%%, Rain Chance: %d%%", hour, tempC, humidity, rainChance);
    char hourStr[10];
    sprintf(hourStr, "%02d:00", hour);
    char tempCStr[10];   
    sprintf(tempCStr, "%.1f C", tempC);
    char rainChanceStr[20];
    sprintf(rainChanceStr, "%d%%", rainChance);
    char humidityStr[15];
    sprintf(humidityStr, "%d%%", humidity);

    // Serial.println(idxHour);
    // Serial.println(hourStr);
    // Serial.println(tempCStr);
    // Serial.println(humidityStr);
    // Serial.println(rainChanceStr);

    epaper.textsize = 2;
    epaper.drawRightString(tempCStr, 10 + indent * (i + 1) - 20, 88 + shift, TEXT_FONT);

    epaper.textsize = 3;
    epaper.drawString(hourStr, 10 + indent * i + 10, 125 + shift, TEXT_FONT);

    epaper.textsize = 1;
    epaper.drawRightString("Rain:", 10 + indent * (i + 1) - 70, 170 + shift, 2);
    epaper.drawRightString("Humidity:", 10 + indent * (i + 1) - 70, 205 + shift, 2);

    epaper.textsize = 2;
    epaper.drawRightString(rainChanceStr, 10 + indent * (i + 1) - 20, 170 + shift, TEXT_FONT);
    epaper.drawRightString(humidityStr, 10 + indent * (i + 1) - 20, 205 + shift, TEXT_FONT);

    idxHour = (idxHour + 1) % 24; // Move to the next hour, wrap around at 24
    // Serial.println("here" + String(idxHour) + " " + String(j) + " " + String(i));
  }
};

void ui2(const struct tm &timeinfo, const bool &wifiConnected) {

  int minute = timeinfo.tm_min;
  int minutesSinceFetch = (minute - fetchAqi + 60) % 60;
  if (minutesSinceFetch >= 20 || aqi == -1)
  {
    aqi = fetchAqiDataFromIqair();
    fetchAqi = minute;
    // updateAqiDisplay(aqi);
  }

  updateAqiDisplay(aqi);
  forecasthourly forecast;

  if (wifiConnected)
  {
    bool needFetchWeather = dateDay != timeinfo.tm_mday || fetchWeather == -1;
    if (needFetchWeather)
    {
      forecast = fetchWeatherForecast();
      if (forecast.hour[0] != -1)
      {
        weatherForecast = forecast; // Store in RTC memory
        dateDay = timeinfo.tm_mday; // Update the day of month
        strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
        strftime(dayStr, sizeof(dayStr), "%A", &timeinfo);
        fetchWeather = timeinfo.tm_hour; // Update the last fetch time
      }
    }
    else
    {
      forecast = weatherForecast; // Load from RTC memory
    }
  }
  else {
    forecast = weatherForecast; // Load from RTC memory

    if (forecast.hour[0] == -1) {
      // No valid weather data available, show placeholder
      for (int i = 0; i < 36; i++) {
        forecast.hour[i] = -1;
        forecast.tempC[i] = 0.0;
        forecast.humidity[i] = 0;
        forecast.rainChance[i] = 0;
      }
    }
  }

  updateDateDisplay(dateStr);
  updateDayDisplay(dayStr);

  drawline();
  updateWeatherDisplay(forecast, timeinfo.tm_hour);
  lastUpdateTime(timeinfo);

  updateBatteryDisplay();
  epaper.update();
}