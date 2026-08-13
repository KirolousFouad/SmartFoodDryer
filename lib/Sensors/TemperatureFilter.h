#ifndef TEMPERATURE_FILTER_H
#define TEMPERATURE_FILTER_H

#include <Arduino.h>
#include <math.h>

class TemperatureFilter
{
public:

    TemperatureFilter();

    void begin(
        float initialValue = NAN
    );

    float update(
        float newValue
    );

    float getValue() const;

    void reset(
        float value = NAN
    );

private:

    float filteredValue;

    bool initialized;

    // 0.0 = very smooth / slow
    // 1.0 = no filtering
    float alpha;
};

#endif