#include "api.h"
#include "secrets.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


int parseAqiDataFromAqicn(const String &json)
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    Serial.print("JSON deserialization failed: ");
    Serial.println(error.c_str());
    return -1;
  }

  if (doc["status"] == "ok")
  {
    int aqi = doc["data"]["aqi"];
    Serial.printf("Current AQI: %d\n", aqi);
    return aqi;
  }
  else
  {
    Serial.println("Failed to get AQI data");
    return -1;
  }
}

int fetchAqiDataFromAqicn()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected");
    return -1;
  }

  HTTPClient http;
  String url = String("https://api.waqi.info/feed/bangkok/?token=") + aqicnToken;
  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0)
  {
    String payload = http.getString();
    Serial.println("AQI Data fetched.");
    // Serial.println(payload);
    int aqi = parseAqiDataFromAqicn(payload);
    http.end();
    return aqi;
  }
  else
  {
    Serial.print("Error on HTTP request: ");
    Serial.println(httpResponseCode);
    http.end();
    return -1;
  }
}

int parseAqiDataFromIqair(const String &json)
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    Serial.print("JSON deserialization failed: ");
    Serial.println(error.c_str());
    return -1;
  }

  if (doc["status"] == "success")
  {
    int aqi = doc["data"]["current"]["pollution"]["aqius"];
    Serial.printf("Current AQI: %d\n", aqi);
    return aqi;
  }
  else
  {
    Serial.println("Failed to get AQI data");
    return -1;
  }
}

int fetchAqiDataFromIqair()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected");
    return -1;
  }

  HTTPClient http;
  String cityEncoded = String(city);
  cityEncoded.replace(" ", "%20");
  String stateEncoded = String(state);
  stateEncoded.replace(" ", "%20");
  String countryEncoded = String(country);
  countryEncoded.replace(" ", "%20");

  String url = String("http://api.airvisual.com/v2/city?city=") + cityEncoded + String("&state=") + stateEncoded + String("&country=") + countryEncoded + String("&key=") + iqairToken;
  // Serial.println(url);
  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0)
  {
    String payload = http.getString();
    Serial.println("AQI Data fetched.");
    // Serial.println(payload);
    int aqi = parseAqiDataFromIqair(payload);
    http.end();
    return aqi;
  }
  else
  {
    Serial.print("Error on HTTP request: ");
    Serial.println(httpResponseCode);
    http.end();
    return -1;
  }
}

forecasthourly fetchWeatherForecast()
{
  forecasthourly forecast;
  forecast.hour[0] = -1; // Initialize with invalid data to indicate failure

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected");
    return forecast;
  }

  HTTPClient http;
  String url = String("http://api.weatherapi.com/v1/forecast.json?key=") + weatherApiKey + String("&q=") + latlon + String("&days=2");
  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0)
  {
    String payload = http.getString();
    Serial.println("Weather forecast data fetched.");
    // Serial.println(payload);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
      Serial.print("JSON deserialization failed: ");
      Serial.println(error.c_str());
      return forecast;
    }
    JsonArray hourly = doc["forecast"]["forecastday"][0]["hour"].as<JsonArray>();
    JsonArray nextHourly = doc["forecast"]["forecastday"][1]["hour"].as<JsonArray>();
    for (size_t i = 0; i < hourly.size() && i < 24; i++)
    {
      forecast.hour[i] = hourly[i]["time"].as<String>().substring(11, 13).toInt();
      forecast.tempC[i] = hourly[i]["temp_c"].as<float>();
      forecast.humidity[i] = hourly[i]["humidity"].as<int>();
      forecast.rainChance[i] = hourly[i]["chance_of_rain"].as<int>();
      // printf("Hour: %d, Temp: %.1f C, Humidity: %d%%, Rain Chance: %d%%\n", forecast.hour[i], forecast.tempC[i], forecast.humidity[i], forecast.rainChance[i]);
    }
    for (size_t i = 0; i < nextHourly.size() && i < 12; i++)
    {
      forecast.hour[24 + i] = nextHourly[i]["time"].as<String>().substring(11, 13).toInt();
      forecast.tempC[24 + i] = nextHourly[i]["temp_c"].as<float>();
      forecast.humidity[24 + i] = nextHourly[i]["humidity"].as<int>();
      forecast.rainChance[24 + i] = nextHourly[i]["chance_of_rain"].as<int>();
      // printf("Hour: %d, Temp: %.1f C, Humidity: %d%%, Rain Chance: %d%%\n", forecast.hour[24 + i], forecast.tempC[24 + i], forecast.humidity[24 + i], forecast.rainChance[24 + i]);
    }

    http.end();
    return forecast;
  }
  else
  {
    Serial.print("Error on HTTP request: ");
    Serial.println(httpResponseCode);
    http.end();
    return forecast;
  }
}