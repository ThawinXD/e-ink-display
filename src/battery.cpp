#include "battery.h"
#include <Arduino.h>

float readBatteryVoltage()
{
  digitalWrite(ADC_EN, HIGH); // Enable ADC voltage divider
  delay(10);                  // Wait for voltage to stabilize
  int sum = 0;
  // Read multiple samples for better accuracy
  for (int i = 0; i < 10; i++)
  {
    sum += analogRead(BATTERY_ADC);
    delay(2);
  }
  int adcValue = sum / 10;

  // Calculate actual battery voltage
  // Formula: voltage = (ADC_value / 4095) * 3.3V * divider_ratio
  float voltage = (adcValue / 4095.0) * 3.3 * VOLTAGE_DIVIDER_RATIO;

  digitalWrite(ADC_EN, LOW); // Disable ADC voltage divider
  return voltage;
}
