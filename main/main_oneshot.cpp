#include <vector>

#include "Arduino.h"
#include "esp_log.h"
#include "Preferences.h"
#include "esp_pm.h"
#include "esp_adc/adc_oneshot.h"

#include "pins.h"
#include "blue.h"

void collectDataTask(void *shit)
{

    adc_channel_t adcChannels[] = {ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_2, ADC_CHANNEL_3};

    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t adc_init_cfg = {
        .unit_id = ADC_UNIT_1,

    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_init_cfg, &adc1_handle));
    adc_oneshot_chan_cfg_t adc_ch_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };

    for (auto ch : adcChannels)
    {
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ch, &adc_ch_cfg));
    }

    while (1)
    {
        vTaskDelay(20);
        if (!blue::isConnected())
            continue;

        std::vector<uint8_t> analogValues{1, 14, 5, 14};
        for (uint8_t i = 0; i < 2; i++)
        {
            int val;
            ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, adcChannels[i], &val));
            printf("Channel %d: %d\n", adcChannels[i], val);
            analogValues[i] = (uint8_t)(val >> 4); // 10 bits raw data
        }
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
