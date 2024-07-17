#include <Arduino.h>
#include "anal.h"

#include "pins.h"

namespace anal
{
    static bool adcStarted = false;
    static volatile bool adcFinished = false;

    uint8_t rowPins[] = {PIN_ROW1, PIN_ROW2, PIN_ROW3, PIN_ROW4};
    void init()
    {
        analogContinuousSetAtten(ADC_11db);
        analogContinuous(rowPins, 4, 5, 10000, []
                         { adcFinished = true; });
    }

    void start()
    {
        if (!adcStarted)
        {
            analogContinuousStart();
            adcStarted = true;
        }
    }

    void stop()
    {
        if (adcStarted)
        {
    // ESP_ERROR_CHECK(adc_continuous_stop(adc_handle[ADC_UNIT_1].adc_continuous_handle));
            analogContinuousStop();
            adcStarted = false;
        }
    }

    bool read(std::vector<uint8_t> *result)
    {
        if (!adcFinished)
            return false;

        adc_continuous_data_t *anaContinousResults;
        analogContinuousRead(&anaContinousResults, 0);
        adcFinished = false;

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
        return true;
    }

} // namespace anal
