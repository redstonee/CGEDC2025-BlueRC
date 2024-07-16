#include <Arduino.h>

#include "anal.h"
#include "pins.h"


namespace anal
{
    static bool adcStarted = false;
    static adc_continuous_handle_t adcHandle;

    const uint8_t rowPins[] = {PIN_ROW1, PIN_ROW2, PIN_ROW3, PIN_ROW4};
    void init(adc_continuous_callback_t cb)
    {
        adc_continuous_handle_cfg_t adc_config = {
            .max_store_buf_size = 1024,
            .conv_frame_size = 4 * 5 * SOC_ADC_DIGI_RESULT_BYTES,
        };

        ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &adcHandle));

        adc_continuous_config_t adcConfig{
            .pattern_num = 4,
            .sample_freq_hz = 20000,
            .conv_mode = ADC_CONV_SINGLE_UNIT_1,
            .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
        };

        adc_digi_pattern_config_t digiConfig[4];
        for (uint8_t i = 0; i < 4; i++)
        {
            adc_unit_t u;
            adc_channel_t ch;
            adc_continuous_io_to_channel(rowPins[i], &u, &ch);
            digiConfig[i].atten = ADC_ATTEN_DB_12;
            digiConfig[i].bit_width = ADC_BITWIDTH_12;
            digiConfig[i].channel = ch;
            digiConfig[i].unit = u;
        }
        adcConfig.adc_pattern = digiConfig;
        ESP_ERROR_CHECK(adc_continuous_config(adcHandle, &adcConfig));

        adc_continuous_evt_cbs_t cbs{
            .on_conv_done = cb,
        };
        ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(adcHandle, &cbs, nullptr));

        // analogContinuousSetAtten(ADC_11db);
        // analogContinuous(rowPins, 4, 5, 10000, cb);
    }

    void start()
    {
        if (!adcStarted)
        {
            // analogContinuousStart();
            ESP_ERROR_CHECK(adc_continuous_start(adcHandle));
            adcStarted = true;
        }
    }

    void stop()
    {
        if (adcStarted)
        {
            // analogContinuousStop();
            ESP_ERROR_CHECK(adc_continuous_stop(adcHandle));
            adcStarted = false;
        }
    }

    bool read(std::vector<uint8_t> *result)
    {
        // if (!adcFinished)
        //     return false;

        uint8_t adcData[4 * 5 * SOC_ADC_DIGI_RESULT_BYTES];
        memset(adcData, 0xcc, sizeof(adcData));
        uint32_t readSize;

        ESP_ERROR_CHECK(adc_continuous_read(adcHandle, adcData, 4 * 5 * SOC_ADC_DIGI_RESULT_BYTES, &readSize, 0));

        for (int i = 0; i < readSize; i += SOC_ADC_DIGI_RESULT_BYTES)
        {
            auto p = (adc_digi_output_data_t *)&adcData[i];
            auto chan_num = p->type2.channel;
            auto data = p->type2.data;

            /* Check the channel number validation, the data is invalid if the channel num exceed the maximum channel */
            if (chan_num >= SOC_ADC_CHANNEL_NUM(0))
            {
                log_e("Invalid data [%d_%d]", chan_num, data);
                return false;
            }

            int pin;
            adc_continuous_channel_to_io(ADC_UNIT_1, static_cast<adc_channel_t>(chan_num), &pin);
            switch (pin)
            {
            case PIN_ROW1:
                result->at(0) = data >> 4;
                break;
            case PIN_ROW2:
                result->at(1) = data >> 4;
                break;
            case PIN_ROW3:
                result->at(2) = data >> 4;
                break;
            case PIN_ROW4:
                result->at(3) = data >> 4;
                break;
            default:
                break;
            }
        }

        // adc_continuous_data_t *anaContinousResults;
        // analogContinuousRead(&anaContinousResults, 0);
        // adcFinished = false;

        return true;
    }

} // namespace anal
