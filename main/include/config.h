#pragma once

#include <stdint.h>

// Touch interrupt pin
constexpr auto TCH_INT_PIN = 10;
constexpr auto LCD_BL_PIN = 12;

// PMIC pins
constexpr auto AXP_SCK_PIN = 25;
constexpr auto AXP_SDA_PIN = 22;
constexpr auto AXP_INT_PIN = 8;

// Sleep timeout in milliseconds
constexpr auto SLEEP_TIMEOUT = 30 * 1000;

// Battery capacity in mAh
constexpr auto BATT_CAP = 500;