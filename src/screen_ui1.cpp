#include "screen_ui1.h"
#include "battery.h"
#include "aqi.h"
#include <time.h>

#define TEXT_FONT 1
#define NUM_TEXT_FONTS 7

const long gmtOffset = 25200; // GMT+7 7 * 3600 seconds
const int daylightOffset = 0;

void ui1()
{
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