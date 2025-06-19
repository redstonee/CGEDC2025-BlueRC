#include <NimBLEDevice.h>
#include <Preferences.h>
#include "blue.h"
#include "config.h"

// WARNING: Global variables, use with CAUTION!
QueueHandle_t bleScanUnsavedDeviceQueue = nullptr; // For discovered BLE devices
QueueHandle_t blePairResultQueue = nullptr;        // For pairing results
SemaphoreHandle_t bleScanStartSemaphore = nullptr; // For scan completion synchronization
SemaphoreHandle_t bleScanDoneSemaphore = nullptr;  // For device list update

namespace blue
{
    constexpr auto TAG = "Blue";
    constexpr auto ID_SERVICE_UUID = "fff8";
    constexpr auto CTRL_SERVICE_UUID = "13b528f9-f225-4c8d-a3db-2c9ab927a22e";

    constexpr auto MODE_CHR_UUID = "fff0";
    constexpr auto TEMP_CHR_UUID = "fff1";
    constexpr auto DIR_CHR_UUID = "fff2";
    constexpr auto FAN_CHR_UUID = "fff3";

    static NimBLEScan *bleScanner;

    static TimerHandle_t scanTimerHandle;

    static Preferences pref;
    static constexpr auto KEY_COUNT = "count";
    static constexpr auto KEY_NAME = "name";
    static constexpr auto KEY_ADDRESS = "addr";

    static std::vector<Device> savedDevices; // List of saved devices

    inline Device *findSavedDeviceByAddr(const uint8_t addr[6])
    {
        for (auto &device : savedDevices)
        {
            if (memcmp(device.address, addr, 6) == 0)
            {
                return &device; // Device is already saved
            }
        }
        return nullptr; // Device not found in saved list
    }

    class ScanCallbacks : public NimBLEScanCallbacks
    {

        void onResult(NimBLEAdvertisedDevice *advertisedDevice) override
        {
            if (advertisedDevice->isAdvertisingService(NimBLEUUID(ID_SERVICE_UUID)))
            {
                ESP_LOGI(TAG, "Found AC: %s\n", advertisedDevice->getName().c_str());
                Device device;
                device.name = new char[advertisedDevice->getName().length() + 1];
                for (auto i = 0; i < 6; i++)
                {
                    // It's reversed for some fucking reason, so we reverse it back
                    device.address[i] = advertisedDevice->getAddress().getNative()[5 - i];
                }
                strcpy(device.name, advertisedDevice->getName().c_str());

                auto savedDevice = findSavedDeviceByAddr(device.address);
                if (savedDevice)
                {
                    savedDevice->online = true; // Mark as online if scanned
                }
                else
                {
                    if (xQueueSend(bleScanUnsavedDeviceQueue, &device, 100) != pdTRUE)
                        ESP_LOGE(TAG, "Failed to send unsaved device to queue, queue is full");
                }
            }
        }
        /** Callback to process the results of the completed scan or restart it */
        void onScanEnd(NimBLEScanResults results) override
        {
            xSemaphoreGive(bleScanDoneSemaphore);
            ESP_LOGI(TAG, "Scan Ended");
        }

    } scanCallbacks;

    class ClientCallbacks : public NimBLEClientCallbacks
    {
        void onConnect(NimBLEClient *pClient) override { ESP_LOGI(TAG, "Connected\n"); }

        void onDisconnect(NimBLEClient *pClient, int reason) override
        {
            printf("%s Disconnected, reason = %d\n", pClient->getPeerAddress().toString().c_str(), reason);
        }

        /********************* Security handled here *********************/
        void onPassKeyEntry(const NimBLEConnInfo &connInfo) override
        {
            /**
             * This should prompt the user to enter the passkey displayed
             * on the peer device.
             */
            NimBLEDevice::injectPassKey(connInfo, 114514);
        }

        /** Pairing process complete, we can check the results in connInfo */
        void onAuthenticationComplete(const NimBLEConnInfo &connInfo) override
        {
            auto result = connInfo.isEncrypted();
            xQueueSend(blePairResultQueue, &result, 0);
            auto client = NimBLEDevice::getClientByPeerAddress(connInfo.getAddress());
            NimBLEDevice::deleteClient(client);
        }
    } clientCallbacks;

    /**
     * @brief Starts scanning for BLE devices.
     *
     * @param timerHandle The timer handle for FreeRTOS, not used here.
     *
     * @note This function is a callback for a FreeRTOS timer.
     */
    void startScan(TimerHandle_t timerHandle)
    {
        if (bleScanner->isScanning())
        {
            ESP_LOGW(TAG, "Already scanning");
            return;
        }

        for (auto &dev : savedDevices)
        {
            dev.online = false; // Reset online status for all saved devices
        }

        /** Start scanning for advertisers */
        xSemaphoreGive(bleScanStartSemaphore);
        bleScanner->start(BLE_SCAN_TIME);

        // Test data for testing purposes
        // Device devices[]{
        //     {
        //         .name = "AIR-1",
        //         .address = {0x58, 0xcf, 0x79, 0x02, 0x9d, 0x2a},
        //     },
        //     {
        //         .name = "AIR-2",
        //         .address = {0x58, 0xcf, 0x79, 0x02, 0x9d, 0x1e},
        //     },
        //     {
        //         .name = "AIR-3",
        //         .address = {0x58, 0xcf, 0x79, 0x02, 0x9d, 0x0a},
        //     }};

        // for (auto &dev : devices)
        // {
        //     auto savedDevice = findSavedDeviceByAddr(dev.address);
        //     if (savedDevice)
        //     {
        //         savedDevice->online = true; // Mark as online if scanned
        //     }
        //     else
        //     {
        //         if (xQueueSend(bleScanUnsavedDeviceQueue, &dev, 100) != pdTRUE)
        //             ESP_LOGE(TAG, "Failed to send unsaved device to queue, queue is full");
        //     }
        // }
        // xSemaphoreGive(bleScanDoneSemaphore);
    }

    void pauseScan()
    {
        if (bleScanner->isScanning())
        {
            bleScanner->stop();
        }
        xTimerStop(scanTimerHandle, 0);
    }

    void resumeScan()
    {
        xTimerStart(scanTimerHandle, 0);
    }

    uint8_t getSavedDeviceCount()
    {
        return pref.getUChar(KEY_COUNT, 0);
    }

    void clearSavedDevices()
    {
        pref.putUChar(KEY_COUNT, 0);
    }

    bool addDevice(const char *name, uint8_t address[6])
    {
        if (getSavedDeviceCount() >= MAX_SAVED_DEVICES)
        {
            ESP_LOGW(TAG, "Maximum number of saved devices reached, cannot save more");
            return false;
        }

        uint8_t count = getSavedDeviceCount();
        pref.putUChar(KEY_COUNT, count + 1);
        pref.putString((KEY_NAME + String(count)).c_str(), name);
        pref.putBytes((KEY_ADDRESS + String(count)).c_str(), address, 6);

        Device device;
        device.name = new char[strlen(name) + 1];
        strcpy(device.name, name);
        memcpy(device.address, address, 6);
        savedDevices.push_back(device);
        return true;
    }

    bool loadDevice(const uint8_t index, char *name, uint8_t *address)
    {
        if (index >= getSavedDeviceCount())
        {
            ESP_LOGW(TAG, "Index out of bounds: %d", index);
            return false;
        }
        auto nameKey = KEY_NAME + String(index);
        auto addrKey = KEY_ADDRESS + String(index);

        if (!pref.isKey(nameKey.c_str()) || !pref.isKey(addrKey.c_str()))
        {
            ESP_LOGW(TAG, "Device not found at index: %d", index);
            return false;
        }

        if (!pref.getString(nameKey.c_str(), name, 32))
        {
            ESP_LOGW(TAG, "Failed to read name for device at index: %d", index);
            return false;
        }

        auto bytesRead = pref.getBytes(addrKey.c_str(), address, 6);
        if (bytesRead != 6)
        {
            ESP_LOGW(TAG, "Failed to read address for device at index: %d", index);
            return false;
        }
        ESP_LOGI(TAG, "Loaded device %d: %s (%02x:%02x:%02x:%02x:%02x:%02x)", index, name,
                 address[0], address[1], address[2], address[3], address[4], address[5]);

        return true;
    }

    std::vector<Device> &getDeviceList()
    {
        return savedDevices;
    }

    void init()
    {
        bleScanUnsavedDeviceQueue = xQueueCreate(3, sizeof(Device));

        blePairResultQueue = xQueueCreate(1, sizeof(bool));
        bleScanStartSemaphore = xSemaphoreCreateBinary();
        bleScanDoneSemaphore = xSemaphoreCreateBinary();

        pref.begin("blue", false);
        BLEDevice::init("Blue RC");

        BLEDevice::setSecurityAuth(true, true, true);
        BLEDevice::setSecurityIOCap(BLE_HS_IO_KEYBOARD_ONLY);

        bleScanner = NimBLEDevice::getScan();

        /** Set the callbacks to call when scan events occur, no duplicates */
        bleScanner->setScanCallbacks(&scanCallbacks, false);

        /** Set scan interval (how often) and window (how long) in milliseconds */
        bleScanner->setInterval(1349);
        bleScanner->setWindow(499);

        bleScanner->setActiveScan(false);
        // bleScanner->start(BLE_SCAN_TIME);

        startScan(nullptr); // Start scanning immediately

        scanTimerHandle = xTimerCreate("BLE Scan Timer", pdMS_TO_TICKS(BLE_SCAN_PERIOD), pdTRUE, nullptr, startScan);
        xTimerStart(scanTimerHandle, 0);

        for (auto i = 0; i < getSavedDeviceCount(); i++)
        {
            Device device;
            char name[32];
            uint8_t address[6];
            if (loadDevice(i, name, address))
            {
                device.name = new char[strlen(name) + 1];
                strcpy(device.name, name);
                memcpy(device.address, address, 6);
                savedDevices.push_back(device);
            }
        }
    }

    bool isScanning()
    {
        return bleScanner->isScanning();
    }

    NimBLEClient *connectToDevice(const uint8_t address[6])
    {
        NimBLEClient *pClient = nullptr;

        uint8_t shit[6];
        memcpy(shit, address, 6);
        NimBLEAddress addr(shit);

        /** Check if we have a client we should reuse first **/
        if (NimBLEDevice::getClientListSize())
        {
            /** Special case when we already know this device, we send false as the
             *  second argument in connect() to prevent refreshing the service database.
             *  This saves considerable time and power.
             */

            pClient = NimBLEDevice::getClientByPeerAddress(addr);
            if (pClient)
            {
                if (!pClient->connect(addr, false))
                {
                    printf("Reconnect failed\n");
                    return nullptr;
                }
                printf("Reconnected client\n");
            }
            /** We don't already have a client that knows this device,
             *  we will check for a client that is disconnected that we can use.
             */
            else
            {
                pClient = NimBLEDevice::getDisconnectedClient();
            }
        }

        /** No client to reuse? Create a new one. */
        if (!pClient)
        {
            if (NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS)
            {
                printf("Max clients reached - no more connections available\n");
                return nullptr;
            }

            pClient = NimBLEDevice::createClient();

            printf("New client created\n");

            pClient->setClientCallbacks(&clientCallbacks, false);
            /** Set initial connection parameters: These settings are 15ms interval, 0 latency, 120ms timout.
             *  These settings are safe for 3 clients to connect reliably, can go faster if you have less
             *  connections. Timeout should be a multiple of the interval, minimum is 100ms.
             *  Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 12 * 10ms = 120ms timeout
             */
            pClient->setConnectionParams(6, 6, 0, 15);
            /** Set how long we are willing to wait for the connection to complete (milliseconds), default is 30000. */
            pClient->setConnectTimeout(BLE_CONN_TIMEOUT);

            if (!pClient->connect(addr))
            {
                /** Created a client but failed to connect, don't need to keep it as it has no data */
                NimBLEDevice::deleteClient(pClient);
                printf("Failed to connect, deleted client\n");
                return nullptr;
            }
        }

        return pClient; // Return the connected client
    }

    bool tryToPairDevice(uint8_t address[6])
    {
        xTaskCreate([](void *arg)
                    {
                        uint8_t address[6];
                        memcpy(address, arg, sizeof(address));
                        auto pClient = connectToDevice(address);
                        if (!pClient)
                        {
                            // Failed to connect
                            ESP_LOGE(TAG, "Failed to connect to device while pairing");
                            NimBLEDevice::deleteClient(pClient);
                            bool foo = false;
                            xQueueSend(blePairResultQueue, &foo, 0);
                        }
                        vTaskDelete(nullptr); // Exit the task
                    },
                    "BLE Connect Task", 4096, address, 5, nullptr);
        return true;
    }

    bool sendControl(const Device &device)
    {
        if (bleScanner->isScanning())
        {
            pauseScan(); // Pause scanning if it's active
        }

        auto pClient = connectToDevice(device.address);
        if (!pClient)
        {
            ESP_LOGW(TAG, "Failed to connect to device");
            return false;
        }

        auto ctrlService = pClient->getService(CTRL_SERVICE_UUID);
        if (!ctrlService)
        {
            ESP_LOGW(TAG, "Control service not found, disconnecting client");
            pClient->disconnect();
            NimBLEDevice::deleteClient(pClient);
            return false;
        }

        auto modeChr = ctrlService->getCharacteristic(MODE_CHR_UUID);
        auto tempChr = ctrlService->getCharacteristic(TEMP_CHR_UUID);
        auto dirChr = ctrlService->getCharacteristic(DIR_CHR_UUID);
        auto fanChr = ctrlService->getCharacteristic(FAN_CHR_UUID);

        bool result = true;
        result &= modeChr->writeValue(static_cast<uint8_t>(device.mode));
        result &= tempChr->writeValue(device.temperature);
        result &= dirChr->writeValue(static_cast<uint8_t>(device.direction));
        result &= fanChr->writeValue(static_cast<uint8_t>(device.speed));

        pClient->disconnect();               // Disconnect the client after use
        NimBLEDevice::deleteClient(pClient); // Delete the client to free resources
        return result;
    }

    bool readStatus(Device &device)
    {
        if (bleScanner->isScanning())
        {
            pauseScan(); // Pause scanning if it's active
        }

        // auto pClient = NimBLEDevice::createClient();
        // pClient->setClientCallbacks(&clientCallbacks, false);

        // /**
        //  *  Set initial connection parameters:
        //  *  These settings are safe for 3 clients to connect reliably, can go faster if you have less
        //  *  connections. Timeout should be a multiple of the interval, minimum is 100ms.
        //  *  Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 150 * 10ms = 1500ms timeout
        //  */
        // pClient->setConnectionParams(12, 12, 0, 150);

        // /** Set how long we are willing to wait for the connection to complete (milliseconds), default is 30000. */
        // pClient->setConnectTimeout(5 * 1000);

        // uint8_t address[6];
        // memcpy(address, device.address, 6);
        // if (!pClient->connect(NimBLEAddress(address), true))
        // {
        //     /** Created a client but failed to connect, don't need to keep it as it has no data */
        //     NimBLEDevice::deleteClient(pClient);
        //     ESP_LOGW(TAG, "Failed to connect, deleted client\n");
        //     return false;
        // }

        auto pClient = connectToDevice(device.address);
        if (!pClient)
        {
            ESP_LOGW(TAG, "Failed to connect to device");
            return false;
        }

        auto ctrlService = pClient->getService(CTRL_SERVICE_UUID);
        if (!ctrlService)
        {
            ESP_LOGW(TAG, "Control service not found, disconnecting client");
            NimBLEDevice::deleteClient(pClient);
            return false;
        }

        auto modeChr = ctrlService->getCharacteristic(MODE_CHR_UUID);
        auto tempChr = ctrlService->getCharacteristic(TEMP_CHR_UUID);
        auto dirChr = ctrlService->getCharacteristic(DIR_CHR_UUID);
        auto fanChr = ctrlService->getCharacteristic(FAN_CHR_UUID);

        device.mode = static_cast<Device::DeviceMode>(modeChr->readValue<uint8_t>());
        device.temperature = tempChr->readValue<uint8_t>();
        device.direction = static_cast<Device::Direction>(dirChr->readValue<uint8_t>());
        device.speed = static_cast<Device::FanSpeed>(fanChr->readValue<uint8_t>());

        NimBLEDevice::deleteClient(pClient); // Delete the client to free resources
        return true;
    }

} // namespace blue
