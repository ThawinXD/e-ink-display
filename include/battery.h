#ifndef BATTERY_H
#define BATTERY_H

#define BATTERY_ADC A0
#define ADC_EN 6
#define VOLTAGE_DIVIDER_RATIO 2.0 // 100k and 100k resistors

float readBatteryVoltage();

#endif // BATTERY_H
