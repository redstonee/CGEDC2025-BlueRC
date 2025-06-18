#pragma once

#include "WString.h"
#include <stdint.h>

struct Device
{
    enum DeviceMode : uint8_t
    {
        MODE_OFF = 0,
        MODE_HEAT,
        MODE_COOL,
        MODE_DRY,
        MODE_FAN,

        NUM_MODES, // Not a mode, used for counting purposes
    };

    enum Direction : uint8_t
    {
        DIR_SWING = 0, // Swinging
        DIR_UP,        // Up direction,
        DIR_MIDDLE,    // Middle direction
        DIR_DOWN,      // Down direction
    };

    enum FanSpeed : uint8_t
    {
        SPD_AUTO = 0,
        SPD_LOW,
        SPD_MEDIUM,
        SPD_HIGH,
    };

     char *name;
     uint8_t address[6];

    uint8_t temperature;
    DeviceMode mode;
    Direction direction;
    FanSpeed speed;
    bool online = false;

    bool selected = false; // Indicates if the device is selected in the UI
};

inline const char *deviceModeToString(Device::DeviceMode mode)
{
    switch (mode)
    {
    case Device::MODE_OFF:
        return "Off";
    case Device::MODE_HEAT:
        return "Heat";
    case Device::MODE_COOL:
        return "Cool";
    case Device::MODE_DRY:
        return "Dry";
    case Device::MODE_FAN:
        return "Fan";
    default:
        return "Unknown";
    }
}
