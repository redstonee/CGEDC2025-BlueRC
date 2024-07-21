#include <Arduino.h>
#include "anal.h"

#include "pins.h"

namespace anal
{
    static TaskHandle_t _taskToNotify;

    uint8_t rowPins[] = {PIN_ROW1, PIN_ROW2, PIN_ROW3, PIN_ROW4};

    void init(TaskHandle_t taskToNotify)
    {
        _taskToNotify = taskToNotify;
        analogContinuousSetAtten(ADC_11db);
        analogContinuous(rowPins, 4, 5, 20000, []
                         { xTaskNotifyGive(_taskToNotify); });
    }

    void read(std::vector<uint8_t> *result)
    {
        adc_continuous_data_t *anaContinousResults;
        analogContinuousRead(&anaContinousResults, 0);

        for (uint8_t i = 0; i < 4; i++)
        {
            switch (anaContinousResults[i].pin)
            {
            case PIN_ROW1:
                result->at(0) = anaContinousResults[i].avg_read_raw >> 4;
                break;
            case PIN_ROW2:
                result->at(1) = anaContinousResults[i].avg_read_raw >> 4;
                break;
            case PIN_ROW3:
                result->at(2) = anaContinousResults[i].avg_read_raw >> 4;
                break;
            case PIN_ROW4:
                result->at(3) = anaContinousResults[i].avg_read_raw >> 4;
                break;
            default:
                break;
            }
        }
    }

} // namespace anal
