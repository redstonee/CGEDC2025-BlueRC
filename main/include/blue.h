#include <Arduino.h>

namespace blue
{
    /**
     * @brief Initializes the bluetooth stack.
     * 
     */
    void init();

    /**
     * @brief Sends data over BLE.
     * 
     * @param data The data string to send.
     */
    void send(String data);

    /**
     * @brief Sends data over BLE.
     * 
     * @param data The data vector to send.
     */
    void send(std::vector<uint8_t> data);

    /**
     * @brief Checks if a device is connected.
     * 
     * @return true if a device is connected.
     */
    bool isConnected();
}