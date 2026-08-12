#include "WeightSensor.h"
#include <math.h>

// =========================================================
// CONSTRUCTOR
// =========================================================

WeightSensor::WeightSensor(
    uint8_t dataPin,
    uint8_t clockPin
)
    : loadCell(dataPin, clockPin),
      weight(0.0f),
      ready(false),
      taring(false),
      calibrationFactor(-25.00f)
{
}

// =========================================================
// BEGIN
// =========================================================

void WeightSensor::begin()
{
    Serial.println();
    Serial.println("==============================");
    Serial.println(" LOAD CELL INITIALIZATION");
    Serial.println("==============================");

    // -------------------------------------------------
    // Start HX711
    // -------------------------------------------------

    loadCell.begin();

    Serial.println("[WEIGHT] HX711 started");

    // -------------------------------------------------
    // Calibration factor
    // -------------------------------------------------

    loadCell.setCalFactor(
        calibrationFactor
    );

    Serial.print(
        "[WEIGHT] Calibration factor: "
    );

    Serial.println(
        calibrationFactor,
        4
    );

    // -------------------------------------------------
    // Initial tare
    // -------------------------------------------------

    Serial.println(
        "[WEIGHT] Starting tare..."
    );

    loadCell.start(
        2000,
        true
    );

    // -------------------------------------------------
    // Check initialization
    // -------------------------------------------------

    if (loadCell.getTareTimeoutFlag())
    {
        Serial.println(
            "[WEIGHT] ERROR: HX711 tare timeout"
        );

        ready = false;
        taring = false;

        return;
    }

    if (loadCell.getSignalTimeoutFlag())
    {
        Serial.println(
            "[WEIGHT] ERROR: HX711 signal timeout"
        );

        ready = false;
        taring = false;

        return;
    }

    // -------------------------------------------------
    // Ready
    // -------------------------------------------------

    ready = true;
    taring = false;
    weight = 0.0f;

    Serial.println(
        "[WEIGHT] HX711 ready"
    );

    Serial.println(
        "[WEIGHT] Load cell tared"
    );

    Serial.println("==============================");
}

// =========================================================
// UPDATE
// =========================================================

void WeightSensor::update()
{
    if (!ready)
    {
        return;
    }

    // -------------------------------------------------
    // Update HX711
    // -------------------------------------------------

    if (!loadCell.update())
    {
        return;
    }

    // -------------------------------------------------
    // TARE IN PROGRESS
    // -------------------------------------------------

    if (taring)
    {
        if (loadCell.getTareStatus())
        {
            taring = false;
            weight = 0.0f;

            Serial.println(
                "[WEIGHT] Tare complete"
            );
        }

        return;
    }

    // -------------------------------------------------
    // READ WEIGHT
    // -------------------------------------------------

    float newWeight =
        loadCell.getData();

    // -------------------------------------------------
    // Validate
    // -------------------------------------------------

    if (isnan(newWeight))
    {
        Serial.println(
            "[WEIGHT] Invalid reading"
        );

        return;
    }

    // -------------------------------------------------
    // Remove tiny negative values
    // -------------------------------------------------

    if (newWeight < 0.0f &&
        newWeight > -10.0f)
    {
        newWeight = 0.0f;
    }

    // -------------------------------------------------
    // Store weight
    // -------------------------------------------------

    weight = newWeight;

    // -------------------------------------------------
    // SERIAL DIAGNOSTICS
    // -------------------------------------------------

    Serial.print(
        "[WEIGHT] "
    );

    Serial.print(
        weight,
        2
    );

    Serial.println(
        " g"
    );
}

// =========================================================
// TARE
// =========================================================

void WeightSensor::tare()
{
    if (!ready)
    {
        Serial.println(
            "[WEIGHT] Cannot tare: sensor not ready"
        );

        return;
    }

    if (taring)
    {
        return;
    }

    Serial.println(
        "[WEIGHT] Starting tare..."
    );

    loadCell.tareNoDelay();

    taring = true;
}

// =========================================================
// SET CALIBRATION FACTOR
// =========================================================

void WeightSensor::setCalibrationFactor(
    float factor
)
{
    if (factor == 0.0f)
    {
        Serial.println(
            "[WEIGHT] ERROR: Invalid calibration factor"
        );

        return;
    }

    calibrationFactor =
        factor;

    loadCell.setCalFactor(
        calibrationFactor
    );

    Serial.print(
        "[WEIGHT] Calibration factor set to: "
    );

    Serial.println(
        calibrationFactor,
        4
    );
}

// =========================================================
// GET CALIBRATION FACTOR
// =========================================================

float WeightSensor::getCalibrationFactor() const
{
    return calibrationFactor;
}

// =========================================================
// GET WEIGHT
// =========================================================

float WeightSensor::getWeight() const
{
    return weight;
}

// =========================================================
// READY STATUS
// =========================================================

bool WeightSensor::isReady() const
{
    return ready;
}

// =========================================================
// TARE STATUS
// =========================================================

bool WeightSensor::isTaring() const
{
    return taring;
}