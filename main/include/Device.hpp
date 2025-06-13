#pragma once

#include "WString.h"
#include <stdint.h>

enum class DeviceMode : uint8_t
{
    OFF = 0,
    HEAT,
    COOL,
    DRY,
    FAN,
};

enum class FanDirection : uint8_t
{
    SWING = 0, // Swinging
    UP,        // Up direction,
    MIDDLE,    // Middle direction
    DOWN,      // Down direction
};

enum class FanSpeed : uint8_t
{
    AUTO = 0,
    LOW,
    MEDIUM,
    HIGH,
};

struct Device
{
    String name;
    uint8_t temperature;
    DeviceMode mode;
    FanDirection direction;
    FanSpeed speed;

    bool selected = false; // Indicates if the device is selected in the UI
};
