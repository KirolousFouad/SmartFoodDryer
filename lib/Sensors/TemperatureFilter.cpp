#include "TemperatureFilter.h"

TemperatureFilter::TemperatureFilter()
    : filteredValue(NAN),
      initialized(false),
      alpha(0.20f)
{
}

// =====================================================
// BEGIN
// =====================================================

void TemperatureFilter::begin(float initialValue)
{
    filteredValue = initialValue;

    initialized = !isnan(initialValue);
}

// =====================================================
// UPDATE
// =====================================================

float TemperatureFilter::update(float newValue)
{
    if (isnan(newValue))
    {
        return filteredValue;
    }

    // First valid sample
    if (!initialized)
    {
        filteredValue = newValue;

        initialized = true;

        return filteredValue;
    }

    // Exponential moving average
    filteredValue =
        filteredValue +
        alpha *
        (newValue - filteredValue);

    return filteredValue;
}

// =====================================================
// GET VALUE
// =====================================================

float TemperatureFilter::getValue() const
{
    return filteredValue;
}

// =====================================================
// RESET
// =====================================================

void TemperatureFilter::reset(float value)
{
    filteredValue = value;

    initialized = !isnan(value);
}