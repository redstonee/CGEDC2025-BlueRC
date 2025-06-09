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
    class SvrCallbacks : public BLEServerCallbacks
    {
        void onConnect(BLEServer *pServer, BLEConnInfo &connInfo)
        {
            pServer->stopAdvertising();
            deviceConnected = true;
        };

        void onDisconnect(BLEServer *pServer, BLEConnInfo &connInfo, int reason)
        {
            pServer->startAdvertising();
            deviceConnected = false;
        }
    };

    class CharCallbacks : public BLECharacteristicCallbacks
    {
        void onWrite(BLECharacteristic *pCharacteristic, BLEConnInfo &connInfo)
        {
            std::string rxValue = pCharacteristic->getValue();

            if (rxValue.length() > 0)
            {
                printf("Received: %s\n", rxValue.c_str());
            }
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

        // Create the BLE Server
        static auto pServer = BLEDevice::createServer();
        pServer->setCallbacks(new SvrCallbacks());

        // Create the BLE Service
        static auto pService = pServer->createService(SERVICE_UUID);

        // Create a BLE Characteristic for sending data
        pDataChar = pService->createCharacteristic(
            CHARACTERISTIC_UUID_TX, NIMBLE_PROPERTY::NOTIFY);

        // Create a BLE Characteristic for setting name
        BLECharacteristic *pRenameChar = pService->createCharacteristic(
            CHARACTERISTIC_UUID_RX, NIMBLE_PROPERTY::WRITE);

        pRenameChar->setCallbacks(new CharCallbacks());

        // Start the service
        pService->start();

        // Start advertising
        pServer->getAdvertising()->addServiceUUID(pService->getUUID());
        pServer->startAdvertising();
        printf("Waiting a client connection to notify...\n");
    }

} // namespace blue
