#include <vector>

#include "Arduino.h"
#include "esp_log.h"
#include "Preferences.h"
#include "esp_pm.h"

#include "TFT_eSPI.h"

#include "config.h"
#include "blue.h"

extern "C" void app_main()
{
    initArduino();

        // Configure dynamic frequency scaling
    // automatic light sleep is enabled
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 160,
        .min_freq_mhz = 16,
        .light_sleep_enable = true,
    };
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));

    TFT_eSPI screen;
    screen.begin();

    blue::init();
}
