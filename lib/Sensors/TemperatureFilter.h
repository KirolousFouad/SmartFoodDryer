#ifndef TEMPERATURE_FILTER_H
#define TEMPERATURE_FILTER_H

class TemperatureFilter
{
public:
    TemperatureFilter();

    void begin(float initialValue = 0.0f);

    float update(float newValue);

    float getValue() const;

    void reset(float value = 0.0f);

private:
    float filteredValue;
    bool initialized;

    // 0.0 = very smooth / slow
    // 1.0 = no filtering
    float alpha;
};

#endif