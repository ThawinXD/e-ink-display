#ifndef SCREEN_UI2_H
#define SCREEN_UI2_H

#include <Arduino.h>
#include "driver.h"
#include <TFT_eSPI.h>
#include <time.h>

// External references to variables from main
extern EPaper epaper;

void ui2(const struct tm &timeinfo, const bool &wifiConnected);

#endif // SCREEN_UI2_H