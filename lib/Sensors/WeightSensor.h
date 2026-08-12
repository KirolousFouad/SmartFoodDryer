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

    // =================================================
    // INITIALIZATION
    // =================================================

    void begin();

    // Must be called continuously from Application::update()
    void update();

    // =================================================
    // TARE
    // =================================================

    void tare();

    // =================================================
    // CALIBRATION
    // =================================================

    void setCalibrationFactor(float factor);

    float getCalibrationFactor() const;

    // =================================================
    // WEIGHT
    // =================================================

    // Filtered weight in grams
    float getWeight() const;

    // Last raw HX711 weight in grams
    float getRawWeight() const;

    // =================================================
    // STABILITY
    // =================================================

    // True when weight has remained within the
    // configured stability range.
    bool isStable() const;

    // =================================================
    // STATUS
    // =================================================

    bool isReady() const;

    bool isTaring() const;

private:

    // =================================================
    // HX711
    // =================================================

    HX711_ADC loadCell;

    // =================================================
    // WEIGHT
    // =================================================

    float rawWeight;

    float filteredWeight;

    // =================================================
    // STATUS
    // =================================================

    bool ready;

    bool taring;

    // =================================================
    // CALIBRATION
    // =================================================

    float calibrationFactor;

    // =================================================
    // FILTER
    // =================================================

    float filterAlpha;

    bool filterInitialized;

    // =================================================
    // STABILITY
    // =================================================

    float stabilityRange;

    unsigned long stabilityStartTime;

    unsigned long stabilityTime;

    bool stable;
};

#endif