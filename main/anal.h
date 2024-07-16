#pragma once

#include <vector>
#include <stdint.h>
#include "esp_adc/adc_continuous.h"

namespace anal
{
    void init(adc_continuous_callback_t cb);
    void start();
    void stop();
    bool read(std::vector<uint8_t> *result);
} // namespace anal
