#include "TemperatureFilter.h"

TemperatureFilter::TemperatureFilter()
{
    filteredValue = 0.0f;
    initialized = false;

    // Filtering strength
    // Lower = smoother
    // Higher = faster response
    alpha = 0.20f;
}

void TemperatureFilter::begin(float initialValue)
{
    filteredValue = initialValue;
    initialized = true;
}

float TemperatureFilter::update(float newValue)
{
    // First reading becomes the initial value
    if (!initialized)
    {
        filteredValue = newValue;
        initialized = true;

        return filteredValue;
    }

    // Exponential Moving Average
    filteredValue =
        (alpha * newValue) +
        ((1.0f - alpha) * filteredValue);

    return filteredValue;
}

float TemperatureFilter::getValue() const
{
    return filteredValue;
}

void TemperatureFilter::reset(float value)
{
    filteredValue = value;
    initialized = true;
}