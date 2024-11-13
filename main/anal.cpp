#include <Arduino.h>
#include "anal.h"
#include <esp_adc/adc_oneshot.h>

#include "pins.h"

namespace anal
{
    static TaskHandle_t _taskToNotify;

    static adc_oneshot_unit_handle_t adc1_handle;
    void init(TaskHandle_t taskToNotify)
    {
        adc_oneshot_unit_init_cfg_t init_config1 = {
            .unit_id = ADC_UNIT_1,
        };
        adc_oneshot_new_unit(&init_config1, &adc1_handle);
        adc_oneshot_chan_cfg_t config = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };

        for (auto p : colPins)
        {
            adc_channel_t ch;
            adc_unit_t u;
            ESP_ERROR_CHECK(adc_oneshot_io_to_channel(static_cast<uint8_t>(p), &u, &ch));
            ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ch, &config));
        }
        // analogSetAttenuation(ADC_6db);
        // std::vector<uint8_t> colPins;
        // for (auto p : colPins)
        //     colPins.push_back(static_cast<uint8_t>(p));

        // _taskToNotify = taskToNotify;
        // analogContinuousSetAtten(ADC_6db);
        // analogContinuous(colPins.data(), 4, 5, 30000, []
        //                  { xTaskNotifyGive(_taskToNotify); });
    }

    void read(std::vector<uint8_t> &result, uint8_t offset)
    {
        // adc_continuous_data_t *anaContinousResults;
        // analogContinuousRead(&anaContinousResults, 0);

        // for (uint8_t i = 0; i < 4; i++)
        // {
        //     switch (static_cast<ColPin>(anaContinousResults[i].pin))
        //     {
        //     case ColPin::COL1:
        //         result.at(0 + offset) = anaContinousResults[i].avg_read_raw >> 4;
        //         break;
        //     case ColPin::COL2:
        //         result.at(1 + offset) = anaContinousResults[i].avg_read_raw >> 4;
        //         break;
        //     case ColPin::COL3:
        //         result.at(2 + offset) = anaContinousResults[i].avg_read_raw >> 4;
        //         break;
        //     case ColPin::COL4:
        //         result.at(3 + offset) = anaContinousResults[i].avg_read_raw >> 4;
        //         break;
        //     default:
        //         break;
        //     }
        // }
        for (uint8_t i = 0; i < 4; i++)
        {
            int v;
            adc_oneshot_read(adc1_handle, static_cast<adc_channel_t>(i), &v);
            ESP_LOGI("ANALOG", "%d Read %d", i, v);
            result.at(i + offset) = v >> 4;
            vTaskDelay(100);
        }
    }

} // namespace anal
