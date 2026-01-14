#include "aqi.h"
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