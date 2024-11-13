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
    for (auto p : rowPins)
    {
        digitalWrite(static_cast<uint8_t>(p), 1);
    }
    digitalWrite(static_cast<uint8_t>(row_pin), 0);
}

void collectDataTask(void *shit)
{
    std::vector<uint8_t> analogValues(12);
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(15));
        if (!blue::isConnected())
            continue;

#if TEST_DATASRC
        for (uint8_t i = 0; i < 8; i++)
        {
            analogValues[i] += i*2;
        }

#else
          selectRow(rowPins[0]);
        anal::read(analogValues, 0);
#endif

        blue::send(analogValues);
    }
}

extern "C" void app_main()
{
    initArduino();

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

#if !TEST_DATASRC
    for (auto p : rowPins)
        pinMode(static_cast<uint8_t>(p), OUTPUT);
    anal::init(collectDataTaskHandle);
#endif
}
