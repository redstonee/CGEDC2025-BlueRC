#include <Arduino.h>
#include "Device.hpp"

namespace blue
{
    void init();

    bool isScanning();

    void pauseScan();
    void resumeScan();

    void saveDevice(const Device &device);

    bool connectToDevice(uint8_t address[6]);
}