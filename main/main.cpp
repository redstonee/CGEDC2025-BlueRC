#include <vector>

#include "Arduino.h"
#include "esp_log.h"
#include "Preferences.h"
#include "esp_pm.h"

#include "pins.h"
#include "blue.h"
#include "anal.h"

void collectDataTask(void *shit)
{
    while (1)
    {
        vTaskDelay(pdTICKS_TO_MS(20));
        if (!blue::isConnected())
            continue;

        analogContinuousStart();
        std::vector<uint8_t> analogValues(12);

        // Wait for ADC to wake this shit up
        ulTaskNotifyTake(true, portMAX_DELAY);
        anal::read(&analogValues);
        analogContinuousStop();
        blue::send(analogValues);
    }
}

extern "C" void app_main()
{
    initArduino();

    static auto colPins = {PIN_COL1, PIN_COL2, PIN_COL3, PIN_COL4, PIN_COL5, PIN_COL6};
    for (auto p : colPins)
        pinMode(p, OUTPUT);

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
    xTaskCreate(collectDataTask, "Collect data", 5000, &colPins, 4, &collectDataTaskHandle);
    anal::init(collectDataTaskHandle);
}
