#include <Arduino.h>

namespace blue
{
    void init();

    bool isScanning();

    void pauseScan();
    void resumeScan();

    bool connectToDevice(uint8_t address[6]);
}