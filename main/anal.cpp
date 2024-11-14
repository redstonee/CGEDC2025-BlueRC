#include <Arduino.h>
#include "anal.h"
#include <esp_adc/adc_oneshot.h>

#include "config.h"

namespace anal
{
    static bool adcFinished = false;

    void init()
    {
        analogSetAttenuation(ADC_6db);
        std::vector<uint8_t> tmp;
        for (auto p : colPins)
            tmp.push_back(static_cast<uint8_t>(p));

        analogContinuousSetAtten(ADC_6db);
        analogContinuous(tmp.data(), 4, 8, 60000, []
                         { adcFinished = true; });
    }

    void read(std::vector<uint8_t> &result, uint8_t offset)
    {
        analogContinuousStart();
        adc_continuous_data_t *anaContinousResults;
        while (!adcFinished)
        {
            vTaskDelay(1);
        }
        analogContinuousRead(&anaContinousResults, 0);
        analogContinuousStop();
        adcFinished = false;

        for (uint8_t i = 0; i < 4; i++)
        {
            switch (static_cast<ColPin>(anaContinousResults[i].pin))
            {
            case ColPin::COL1:
                result.at(0 + offset) = anaContinousResults[i].avg_read_raw >> 4;
                break;
            case ColPin::COL2:
                result.at(1 + offset) = anaContinousResults[i].avg_read_raw >> 4;
                break;
            case ColPin::COL3:
                result.at(2 + offset) = anaContinousResults[i].avg_read_raw >> 4;
                break;
            case ColPin::COL4:
                result.at(3 + offset) = anaContinousResults[i].avg_read_raw >> 4;
                break;
            default:
                break;
            }
        }
    }

} // namespace anal
