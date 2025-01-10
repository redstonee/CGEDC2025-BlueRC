#pragma once

#include <vector>
#include <stdint.h>

namespace anal
{
    /**
     * @brief Initializes the ADC for continuous reading.
     * 
     */
    void init();

    /**
     * @brief Reads the analog values from the ADC.
     * 
     * @param result The vector to store the results in.
     * @param offset The offset to start writing the results at.
     */
    void read(std::vector<uint8_t> &result, uint8_t offset);
} // namespace anal
