#ifndef SCREEN_UI1_H
#define SCREEN_UI1_H

#include <Arduino.h>
#include "driver.h"
#include <TFT_eSPI.h>
#include <time.h>

// External references to variables from main
extern EPaper epaper;
extern int fetchAqiHour;
extern int aqi;

void ui1(const struct tm &timeinfo, bool wifiConnected);

#endif // SCREEN_UI1_H