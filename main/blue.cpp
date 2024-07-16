#include <NimBLEDevice.h>
#include <Preferences.h>
#include "blue.h"

namespace blue
{
    static Preferences pref;
    static BLECharacteristic *pTxCharacteristic;
    static bool deviceConnected = false;

    // See the following for generating UUIDs:
    // https://www.uuidgenerator.net/

    constexpr auto SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"; // UART service UUID
    constexpr auto CHARACTERISTIC_UUID_RX = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
    constexpr auto CHARACTERISTIC_UUID_TX = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

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
                pref.putString("name", rxValue.c_str());
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

        pTxCharacteristic->setValue((uint8_t *)data.c_str(), data.length());
        pTxCharacteristic->notify();
    }

    void send(std::vector<uint8_t> data)
    {
        if (!isConnected())
            return;

        pTxCharacteristic->setValue(data);
        pTxCharacteristic->notify();
    }

    void init()
    {
        pref.begin("blue", false);

        auto devName = pref.getString("name", "Stinky Foot");
        BLEDevice::init(devName.c_str());

        static auto pServer = BLEDevice::createServer();
        ;

        // Create the BLE Server
        pServer->setCallbacks(new SvrCallbacks());

        // Create the BLE Service
        static auto pService = pServer->createService(SERVICE_UUID);

        // Create a BLE Characteristic for sending data
        pTxCharacteristic = pService->createCharacteristic(
            CHARACTERISTIC_UUID_TX, NIMBLE_PROPERTY::NOTIFY);

        /***************************************************
         NOTE: DO NOT create a 2902 descriptor
         it will be created automatically if notifications
         or indications are enabled on a characteristic.

         pCharacteristic->addDescriptor(new BLE2902());
        ****************************************************/

        // Create a BLE Characteristic for setting name
        BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
            CHARACTERISTIC_UUID_RX, NIMBLE_PROPERTY::WRITE);

        pRxCharacteristic->setCallbacks(new CharCallbacks());

        // Start the service
        pService->start();

        // Start advertising
        pServer->getAdvertising()->start();
        printf("Waiting a client connection to notify...\n");
    }

} // namespace blue
