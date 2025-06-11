#include <NimBLEDevice.h>
#include <Preferences.h>
#include "blue.h"

namespace blue
{
    static BLECharacteristic *pDataChar;
    static bool deviceConnected = false;

    // See the following for generating UUIDs:
    // https://www.uuidgenerator.net/

    constexpr auto SERVICE_UUID = "2333";
    constexpr auto CHARACTERISTIC_UUID_RX = "6E400002-B5A3-F393-E0A9-114514191981";
    constexpr auto CHARACTERISTIC_UUID_TX = "6E400003-B5A3-F393-E0A9-114514191981";

    /**  None of these are required as they will be handled by the library with defaults. **
     **                       Remove as you see fit for your needs                        */
    class SvrCallbacks : public NimBLEServerCallbacks
    {
        void onConnect(BLEServer *pServer, BLEConnInfo &connInfo) override
        {
            deviceConnected = true;
        };

        void onDisconnect(BLEServer *pServer, BLEConnInfo &connInfo, int reason) override
        {
            deviceConnected = false;
        }

        uint32_t onPassKeyDisplay() override
        {
            // Return a passkey for display, this is just an example
            // You can implement your own logic to generate a passkey
            return 123456; // Example passkey
        }

        void onConfirmPIN(const NimBLEConnInfo &connInfo, uint32_t pin) override
        {
            /** Return false if passkeys don't match. */
            bool matched = (pin == 123456); // Example passkey check
            ESP_LOGI("Blue", "Confirm PIN: %s", matched ? "Matched" : "Not Matched");
            NimBLEDevice::injectConfirmPIN(connInfo, matched);
        };

        void onAuthenticationComplete(const NimBLEConnInfo &connInfo, const std::string &name) override
        {
            ESP_LOGI("Blue", "Authentication complete for %s", name.c_str());
        }
    };

    bool isConnected()
    {
        return deviceConnected;
    }

    void send(String data)
    {
        if (!isConnected())
            return;

        pDataChar->setValue((uint8_t *)data.c_str(), data.length());
        pDataChar->notify();
    }

    void send(std::vector<uint8_t> data)
    {
        if (!isConnected())
            return;

        pDataChar->setValue(data);
        pDataChar->notify();
    }

    void init()
    {
        BLEDevice::init("Blue RC");

        BLEDevice::setSecurityAuth(true, true, true);
        BLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
        // Create the BLE Server
        static auto pServer = BLEDevice::createServer();
        pServer->setCallbacks(new SvrCallbacks());

        // Create the BLE Service
        static auto pService = pServer->createService(SERVICE_UUID);

        // Create a BLE Characteristic for sending data
        pDataChar = pService->createCharacteristic(
            CHARACTERISTIC_UUID_TX, NIMBLE_PROPERTY::NOTIFY);

        // Start the service
        // pService->start();
        // // Start advertising
        // pServer->getAdvertising()->addServiceUUID(pService->getUUID());
        // pServer->startAdvertising();
    }

} // namespace blue
