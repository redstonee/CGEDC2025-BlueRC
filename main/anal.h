#pragma once

#include <vector>
#include <stdint.h>

namespace anal
{
    void init(TaskHandle_t taskToNotify);
    void read(std::vector<uint8_t> *result,uint8_t offset);
} // namespace anal
