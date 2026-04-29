#include "screen_ui1.h"
#include "battery.h"
#include "api.h"
#include <time.h>

#define TEXT_FONT 1
#define NUM_TEXT_FONTS 7

static RTC_DATA_ATTR int dateDay = -1;
static RTC_DATA_ATTR int fetchAqi = -1;
static RTC_DATA_ATTR int aqi = -1;

static void updateDateDisplay(const struct tm &timeinfo)
{
  char dateStr[15];
  strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
  epaper.textsize = 1;
  epaper.drawString(dateStr, 10, 40, NUM_TEXT_FONTS);
  // epaper.updataPartial(10, 40, 200, 40);
};

static void updateDayDisplay(const struct tm &timeinfo)
{
  char dayStr[10];
  strftime(dayStr, sizeof(dayStr), "%A", &timeinfo);
  epaper.textsize = 5;
  epaper.drawRightString(dayStr, 790, 40, TEXT_FONT);
  // epaper.updataPartial(790 - epaper.textWidth(dayStr, TEXT_FONT), 40, epaper.textWidth(dayStr, TEXT_FONT), 60);
};

static void updateTimeDisplay(const struct tm &timeinfo)
{
  char timeStr[10];
  strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
  epaper.textsize = 3;
  epaper.drawCentreString(timeStr, 380, 160, NUM_TEXT_FONTS);
  // epaper.updataPartial(380 - epaper.textWidth(timeStr, NUM_TEXT_FONTS) / 2, 160, epaper.textWidth(timeStr, NUM_TEXT_FONTS), 200);
};

static void updateAqiDisplay(const int &aqi)
{
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
  // epaper.updataPartial(10, 400, 300, 80);
};

static void updateBatteryDisplay()
{
  char batteryStr[30];
  epaper.textsize = 3;
  sprintf(batteryStr, "Battery: %.2f V", readBatteryVoltage());
  epaper.drawRightString(batteryStr, 790, 430, TEXT_FONT);
  // epaper.updataPartial(790 - epaper.textWidth(batteryStr, TEXT_FONT), 430, epaper.textWidth(batteryStr, TEXT_FONT), 40);
};

void ui1(const struct tm &timeinfo, bool wifiConnected)
{
  int minute = timeinfo.tm_min;
  int minutesSinceFetch = (minute - fetchAqi + 60) % 60;
  if (minutesSinceFetch >= 20 || aqi == -1)
  {
    aqi = fetchAqiDataFromIqair();
    fetchAqi = minute;
    // updateAqiDisplay(aqi);
  }
  updateAqiDisplay(aqi);

  updateTimeDisplay(timeinfo);

  // if (dateDay != timeinfo.tm_mday)
  // {
  //   dateDay = timeinfo.tm_mday;
  //   updateDateDisplay(timeinfo);
  //   updateDayDisplay(timeinfo);
  // }
  updateDateDisplay(timeinfo);
  updateDayDisplay(timeinfo);

  updateBatteryDisplay();

  epaper.update();
}