#include <Arduino.h>
#include <vector>
#include "Device.hpp"

namespace blue
{
    void init();

    bool isScanning();

    void pauseScan();
    void resumeScan();

    // uint8_t getSavedDeviceCount();
    bool addDevice(const char *name, uint8_t address[6]);
    std::vector<Device> &getDeviceList();

    // bool loadDevice(const uint8_t index, char *name, uint8_t *address);
    void clearSavedDevices();

    bool sendControl(const Device &device);
    bool readStatus(Device &device);
    
    bool tryToPairDevice(uint8_t address[6]);
}