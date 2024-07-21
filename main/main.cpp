#include <vector>

#include "Arduino.h"
#include "esp_log.h"
#include "Preferences.h"
#include "esp_pm.h"

#include "pins.h"
#include "blue.h"
#include "anal.h"

void selectRow(RowPin row_pin)
{
    for (auto p : RowPins)
    {
        digitalWrite(static_cast<uint8_t>(p), 1);
    }
    digitalWrite(static_cast<uint8_t>(row_pin), 0);
}

void collectDataTask(void *shit)
{
    while (1)
    {
        vTaskDelay(pdTICKS_TO_MS(20));
        if (!blue::isConnected())
            continue;

        std::vector<uint8_t> analogValues(12);

        for (uint8_t i = 0; i < 3; i++)
        {
            selectRow(RowPins[i]);
            analogContinuousStart();
            // Wait for ADC to wake this shit up
            ulTaskNotifyTake(true, portMAX_DELAY);
            anal::read(&analogValues, i * 4);
            analogContinuousStop();
        }

        blue::send(analogValues);
    }
}

extern "C" void app_main()
{
    initArduino();

    for (auto p : RowPins)
        pinMode(static_cast<uint8_t>(p), OUTPUT);

    // Configure dynamic frequency scaling
    // automatic light sleep is enabled
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 48,
        .min_freq_mhz = 8,
        .light_sleep_enable = true,
    };
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));

    blue::init();

    TaskHandle_t collectDataTaskHandle;
    xTaskCreate(collectDataTask, "Collect data", 5000, nullptr, 4, &collectDataTaskHandle);
    anal::init(collectDataTaskHandle);
}
