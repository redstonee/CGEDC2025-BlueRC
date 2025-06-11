#pragma once

#include "Tab.hpp"
#include "WString.h"
#include <vector>

class ControlTab : public Tab
{
public:
    enum class DeviceMode
    {
        OFF = 0,
        HEAT,
        COOL,
        DRY,
        FAN,
    };
    struct Device
    {
        String name;
        uint8_t temperature;
        DeviceMode mode;

        bool selected = false; // Indicates if the device is selected in the UI
    };

private:
    std::vector<Device> _deviceList; // List of saved devices

public:
    ControlTab(lv_obj_t *parent);

    void initTab() override;

    void updateTab() override;
};
