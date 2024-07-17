#pragma once

#include <vector>
#include <stdint.h>

namespace anal
{
    void init();
    void start();
    void stop();
    bool read(std::vector<uint8_t> *result);
} // namespace anal
