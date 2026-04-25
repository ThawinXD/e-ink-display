#ifndef AQI_H
#define AQI_H

#include <Arduino.h>

struct forecasthourly
{
	int hour[36];
	float tempC[36];
	int humidity[36];
	int rainChance[36];
};

int parseAqiDataFromAqicn(const String &json);
int fetchAqiDataFromAqicn();
int parseAqiDataFromIqair(const String &json);
int fetchAqiDataFromIqair();
forecasthourly fetchWeatherForecast();

#endif // AQI_H
