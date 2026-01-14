#ifndef AQI_H
#define AQI_H

#include <Arduino.h>

int parseAqiDataFromAqicn(const String &json);
int fetchAqiDataFromAqicn();
int parseAqiDataFromIqair(const String &json);
int fetchAqiDataFromIqair();

#endif // AQI_H
