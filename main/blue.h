#include <Arduino.h>

namespace blue
{
    void init();
    void send(String data);
    void send(std::vector<uint8_t> data);
    bool isConnected();
}