#ifndef WEIGHT_SENSOR_H
#define WEIGHT_SENSOR_H

#include <Arduino.h>
#include <HX711_ADC.h>

class WeightSensor
{
public:

    WeightSensor(
        uint8_t dataPin,
        uint8_t clockPin
    );

    void begin();

    void update();

    void tare();

    float getWeight() const;

    bool isReady() const;

private:

    HX711_ADC loadCell;

    float weight;

    bool ready;
};

#endif