#include <vector>

#include "Arduino.h"
#include "esp_log.h"
#include "Preferences.h"
#include "esp_pm.h"

#include "config.h"
#include "blue.h"
#include "GFX.h"
#include "Device.hpp"

#include "AXP173.h"

// WARNING: Global variables, use with CAUTION!
QueueHandle_t batteryInfoQueue;
QueueHandle_t deviceControlQueue;

constexpr auto TAG = "Main";

void sleeeeeep()
{
    esp_sleep_enable_ext1_wakeup(1ULL << TCH_INT_PIN, ESP_EXT1_WAKEUP_ANY_LOW);
    esp_deep_sleep_start();
}

extern "C" void app_main()
{
    initArduino();

    // Configure dynamic frequency scaling
    // automatic light sleep is enabled
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 64,
        .min_freq_mhz = 16,
        .light_sleep_enable = false,
    };
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));

    Wire.begin(AXP_SDA_PIN, AXP_SCK_PIN, 4e5); // Initialize I2C with specified SDA and SCL pins
    AXP173 axp(Wire);
    if (!axp.begin())
    { // Initialize AXP173 PMIC
        ESP_LOGE(TAG, "Failed to initialize PMIC");
        while (1)
        {
            delay(1000);
        }
    }
    ESP_LOGI(TAG, "PMIC initialized successfully");

    // Disable unused outputs
    axp.setOutputEnable(axp.DCDC2, false);
    axp.setOutputEnable(axp.LDO2, false);
    axp.setOutputEnable(axp.LDO3, false);
    axp.setOutputEnable(axp.LDO4, false);
    // Set the charge current
    axp.setChargeCurrent(axp.CHG_450mA);
    // Enable Battery ADC channels
    axp.setADCEnable(axp.ADC_BAT_V, true);
    axp.setADCEnable(axp.ADC_BAT_C, true);

    // Attach the interrupt handler for AXP173 and set up IRQs
    static bool axpIRQPending = false;
    attachInterrupt(AXP_INT_PIN, []()
                    { axpIRQPending = true; }, FALLING);

    std::bitset<axp.NUM_IRQn> irqs;
    irqs.set(axp.PEK_SHORT_PRESS_IRQn)
        .set(axp.BAT_CHG_FIN_IRQn);
    axp.enableIRQs(irqs); // Enable VBUS insert and remove interrupts

    // Create a queue for battery info
    batteryInfoQueue = xQueueCreate(3, sizeof(std::tuple<int, int, uint8_t>));
    deviceControlQueue = xQueueCreate(8, sizeof(Device));

    GFX::init();
    blue::init();

    while (1)
    {
        if (axpIRQPending)
        {
            auto irqs = axp.getIRQFlags();
            if (irqs[axp.PEK_SHORT_PRESS_IRQn])
            {
                ESP_LOGI(TAG, "PEK short press detected");
                axp.clearIRQFlags(irqs); // Clear the IRQ flags before sleeping
                sleeeeeep();
                // Unreachable, as sleeeeeep() will terminate the program
            }
            if (irqs[axp.BAT_CHG_FIN_IRQn])
            {
                ESP_LOGI(TAG, "Battery charging finished, reset coulometer");
                axp.resetCoulometer();
            }
            axp.clearIRQFlags(irqs); // Clear the IRQ flags after processing
        }

        // Check inactivity
        if (millis() - GFX::getLastTouchTime() > SLEEP_TIMEOUT)
        {
            ESP_LOGI(TAG, "No touch detected for %d s, going to sleep", SLEEP_TIMEOUT / 1000);
            sleeeeeep();
        }

        // Gather battery information and send it to the queue
        auto battLevel = 1 + axp.getCoulometerData() / BATT_CAP;
        if (battLevel > 1)
            battLevel = 1; // Cap battery level to 100%
        else if (battLevel < 0)
            battLevel = 0; // Ensure battery level is not negative

        auto batteryInfo = std::make_tuple<int, int, uint8_t>(
            axp.getBatVoltage(),
            axp.getBatCurrent(),
            battLevel * 100);

        if (xQueueSend(batteryInfoQueue, &batteryInfo, 0) != pdTRUE)
        {
            ESP_LOGW(TAG, "Battery info queue is full, dropping data");
        }

        Device deviceToControl;
        while (xQueueReceive(deviceControlQueue, &deviceToControl, 0))
        {
            ESP_LOGI(TAG, "%s: mode: %d, temperature: %d, speed : %d, direction: %d",
                     deviceToControl.name,
                     static_cast<int>(deviceToControl.mode),
                     deviceToControl.temperature,
                     static_cast<int>(deviceToControl.speed),
                     static_cast<int>(deviceToControl.direction));
        }

        delay(1000);
    }
}