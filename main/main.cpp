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
    anal::init();
    while (1)
    {
        vTaskDelay(pdTICKS_TO_MS(20));
        if (!blue::isConnected())
            continue;

        anal::start();
        std::vector<uint8_t> analogValues(12);

        while (!anal::read(&analogValues))
            ;
        anal::stop();
        blue::send(analogValues);
    }
}

extern "C" void app_main()
{
    initArduino();

    static auto colPins = {PIN_COL1, PIN_COL2, PIN_COL3, PIN_COL4, PIN_COL5, PIN_COL6};

    for (auto p : colPins)
        pinMode(p, OUTPUT);

#if CONFIG_PM_ENABLE
    // Configure dynamic frequency scaling:
    // maximum and minimum frequencies are set in sdkconfig,
    // automatic light sleep is enabled if tickless idle support is enabled.
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 48,
        .min_freq_mhz = 8,
#if CONFIG_FREERTOS_USE_TICKLESS_IDLE
        .light_sleep_enable = true
#endif
    };
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
#endif // CONFIG_PM_ENABLE

    blue::init();

    xTaskCreate(collectDataTask, "Collect data", 5000, &colPins, 4, nullptr);
}
