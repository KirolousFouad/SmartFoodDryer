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

    // Initialization
    void begin();

    // Must be called continuously from Application::update()
    void update();

    // Start tare operation
    void tare();

    // Set calibration factor
    void setCalibrationFactor(float factor);

    // Get calibration factor
    float getCalibrationFactor() const;

    // Get current filtered weight in grams
    float getWeight() const;

    // Check whether HX711 is ready
    bool isReady() const;

    // Check whether tare is currently running
    bool isTaring() const;

private:

    // =================================================
    // HX711
    // =================================================

    HX711_ADC loadCell;

    // =================================================
    // WEIGHT
    // =================================================

    float weight;

    // =================================================
    // STATUS
    // =================================================

    bool ready;
    bool taring;

    // =================================================
    // CALIBRATION
    // =================================================

    float calibrationFactor;
};

#endif
