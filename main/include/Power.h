#pragma once
#include <utility>

namespace Power
{
    void init();

    std::pair<float, float> getBatteryInfo(); // Returns a pair of battery voltage and current
} // namespace Power
