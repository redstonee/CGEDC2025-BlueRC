#include <NimBLEDevice.h>
#include <Preferences.h>
#include <vector>
#include "Device.hpp"
#include "blue.h"
#include "config.h"

// WARNING: Global variables, use with CAUTION!
QueueHandle_t bleScanDeviceQueue = nullptr;        // For discovered BLE devices
QueueHandle_t blePairResultQueue = nullptr;        // For pairing results
SemaphoreHandle_t bleScanStartSemaphore = nullptr; // For scan completion synchronization

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
    class ScanCallbacks : public NimBLEScanCallbacks
    {
        void onResult(NimBLEAdvertisedDevice *advertisedDevice) override
        {
            if (advertisedDevice->isAdvertisingService(NimBLEUUID(ID_SERVICE_UUID)))
            {
                ESP_LOGI(TAG, "Found server: %s\n", advertisedDevice->getName().c_str());
                // Add to discovered devices
                Device device;
                device.name = new char[advertisedDevice->getName().length() + 1];
                memcpy(device.address, advertisedDevice->getAddress().getNative(), sizeof(device.address));
                strcpy(device.name, advertisedDevice->getName().c_str());
                if (xQueueSend(bleScanDeviceQueue, &advertisedDevice, 100) != pdTRUE)
                    ESP_LOGE(TAG, "Failed to send device to queue, queue is full");
            }
        }

        /** Callback to process the results of the completed scan or restart it */
        void onScanEnd(NimBLEScanResults results) override
        {
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

        void onConfirmPIN(const NimBLEConnInfo &connInfo, uint32_t pin) override
        {
            ESP_LOGI(TAG, "The passkey YES/NO number: %" PRIu32 "\n", pin);
            /** Inject false if passkeys don't match. */
            NimBLEDevice::injectConfirmPIN(connInfo, true);
        }

        /** Pairing process complete, we can check the results in connInfo */
        void onAuthenticationComplete(const NimBLEConnInfo &connInfo) override
        {
            if (!connInfo.isEncrypted())
            {
                ESP_LOGW(TAG, "Encrypt connection failed - disconnecting\n");
                /** Find the client with the connection handle provided in connInfo */
                NimBLEDevice::getClientByPeerAddress(connInfo.getAddress())->disconnect();
                return;
            }
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

        /** Start scanning for advertisers */
        xSemaphoreGive(bleScanStartSemaphore);
        bleScanner->start(BLE_SCAN_TIME);
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

    void init()
    {
        bleScanDeviceQueue = xQueueCreate(3, sizeof(Device));
        blePairResultQueue = xQueueCreate(1, sizeof(bool));
        bleScanStartSemaphore = xSemaphoreCreateBinary();

        BLEDevice::init("Blue RC");

        BLEDevice::setSecurityAuth(true, true, true);
        BLEDevice::setSecurityIOCap(BLE_HS_IO_KEYBOARD_ONLY);

        bleScanner = NimBLEDevice::getScan();

        /** Set the callbacks to call when scan events occur, no duplicates */
        bleScanner->setScanCallbacks(&scanCallbacks, false);

        /** Set scan interval (how often) and window (how long) in milliseconds */
        bleScanner->setInterval(100);
        bleScanner->setWindow(100);

        bleScanner->setActiveScan(true);

        scanTimerHandle = xTimerCreate("BLE Scan Timer", pdMS_TO_TICKS(BLE_SCAN_PERIOD), pdTRUE, nullptr, startScan);
        xTimerStart(scanTimerHandle, 0);

        // auto pClient = NimBLEDevice::createClient();

        // printf("New client created\n");

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

        // if (!pClient->connect(discoveredDevices[0], true))
        // {
        //     /** Created a client but failed to connect, don't need to keep it as it has no data */
        //     NimBLEDevice::deleteClient(pClient);
        //     ESP_LOGW(TAG, "Failed to connect, deleted client\n");
        //     return;
        // }

        // auto ctrlService = pClient->getService(CTRL_SERVICE_UUID);
        // if (!ctrlService)
        // {
        //     ESP_LOGW(TAG, "Control service not found, disconnecting client");
        //     pClient->disconnect();
        //     NimBLEDevice::deleteClient(pClient);
        //     return;
        // }

        // auto modeChr = ctrlService->getCharacteristic(MODE_CHR_UUID);
        // if (!modeChr)
        // {
        //     ESP_LOGW(TAG, "Mode characteristic not found, disconnecting client");
        //     pClient->disconnect();
        //     NimBLEDevice::deleteClient(pClient);
        //     return;
        // }
        // auto modeVal = modeChr->readValue();                    // Read the initial value
        // ESP_LOGI(TAG, "Mode: %u", modeVal.getValue<uint8_t>()); // Get the value as uint8_t
        // modeChr->writeValue<uint8_t>(1);                        // Set the mode to 1

        // pClient->disconnect();               // Disconnect the client after use
        // NimBLEDevice::deleteClient(pClient); // Delete the client to free resources
    }

    bool isScanning()
    {
        return bleScanner->isScanning();
    }

    bool connectToDevice(uint8_t address[6])
    {
        xTaskCreate([](void *arg)
                    {
                        uint8_t address[6];
                        memcpy(address, arg, sizeof(address));

                        auto pClient = NimBLEDevice::createClient();
                        pClient->setClientCallbacks(&clientCallbacks, false);
                        pClient->setConnectionParams(12, 12, 0, 150);
                        /** Set how long we are willing to wait for the connection to complete (milliseconds), default is 30000. */
                        pClient->setConnectTimeout(5 * 1000);
                        auto result = pClient->connect(NimBLEAddress(address));
                        NimBLEDevice::deleteClient(pClient);

                        xQueueSend(blePairResultQueue, &result, 0); // Send failure to queue
                        vTaskDelete(nullptr);                       // Exit the task
                    },
                    "BLE Connect Task", 4096, address, 5, nullptr);
        return true; 
    }

} // namespace blue
