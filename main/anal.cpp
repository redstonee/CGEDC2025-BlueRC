#include <Arduino.h>
#include "anal.h"

#include "pins.h"

namespace anal
{
    static TaskHandle_t _taskToNotify;

    void init(TaskHandle_t taskToNotify)
    {
        std::vector<uint8_t> colPins;
        for (auto p : ColPins)
            colPins.push_back(static_cast<uint8_t>(p));

        _taskToNotify = taskToNotify;
        analogContinuousSetAtten(ADC_11db);
        analogContinuous(colPins.data(), 4, 5, 20000, []
                         { xTaskNotifyGive(_taskToNotify); });
    }

    void read(std::vector<uint8_t> *result, uint8_t offset)
    {
        adc_continuous_data_t *anaContinousResults;
        analogContinuousRead(&anaContinousResults, 0);

        for (uint8_t i = 0; i < 4; i++)
        {
            switch (static_cast<ColPin>(anaContinousResults[i].pin))
            {
            case ColPin::COL1:
                result->at(0 + offset) = anaContinousResults[i].avg_read_raw >> 4;
                break;
            case ColPin::COL2:
                result->at(1 + offset) = anaContinousResults[i].avg_read_raw >> 4;
                break;
            case ColPin::COL3:
                result->at(2 + offset) = anaContinousResults[i].avg_read_raw >> 4;
                break;
            case ColPin::COL4:
                result->at(3 + offset) = anaContinousResults[i].avg_read_raw >> 4;
                break;
            default:
                break;
            }
        }
    }

} // namespace anal
