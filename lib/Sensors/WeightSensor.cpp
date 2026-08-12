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

      rawWeight(0.0f),
      filteredWeight(0.0f),

      ready(false),
      taring(false),

      calibrationFactor(-25.0f),

      filterAlpha(0.20f),
      filterInitialized(false),

      stabilityRange(5.0f),
      stabilityStartTime(0),
      stabilityTime(3000UL),

      stable(false)
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

    Serial.println(
        "[WEIGHT] HX711 started"
    );

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
    // Start tare
    // -------------------------------------------------

    Serial.println(
        "[WEIGHT] Starting tare..."
    );

    loadCell.start(
        2000,
        true
    );

    // -------------------------------------------------
    // Check tare timeout
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

    // -------------------------------------------------
    // Check signal timeout
    // -------------------------------------------------

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
    // Initialize state
    // -------------------------------------------------

    rawWeight = 0.0f;
    filteredWeight = 0.0f;

    filterInitialized = false;

    stabilityStartTime = 0;
    stable = false;

    ready = true;
    taring = false;

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

    // =================================================
    // TARE
    // =================================================

    if (taring)
    {
        if (loadCell.getTareStatus())
        {
            taring = false;

            rawWeight = 0.0f;
            filteredWeight = 0.0f;

            filterInitialized = false;

            stabilityStartTime = 0;
            stable = false;

            Serial.println(
                "[WEIGHT] Tare complete"
            );
        }

        return;
    }

    // =================================================
    // READ RAW WEIGHT
    // =================================================

    float newWeight =
        loadCell.getData();

    // -------------------------------------------------
    // Invalid reading
    // -------------------------------------------------

    if (isnan(newWeight))
    {
        stable = false;
        stabilityStartTime = 0;

        return;
    }

    // -------------------------------------------------
    // Small negative values
    // -------------------------------------------------

    if (newWeight < 0.0f &&
        newWeight > -10.0f)
    {
        newWeight = 0.0f;
    }

    rawWeight = newWeight;

    // =================================================
    // EMA FILTER
    // =================================================

    if (!filterInitialized)
    {
        filteredWeight = rawWeight;

        filterInitialized = true;
    }
    else
    {
        filteredWeight =
            (filterAlpha * rawWeight) +
            ((1.0f - filterAlpha) * filteredWeight);
    }

    // =================================================
    // STABILITY DETECTION
    // =================================================

    float difference =
        fabs(
            rawWeight -
            filteredWeight
        );

    // -------------------------------------------------
    // Weight is currently stable
    // -------------------------------------------------

    if (difference <= stabilityRange)
    {
        if (stabilityStartTime == 0)
        {
            stabilityStartTime =
                millis();
        }

        if (millis() -
            stabilityStartTime >= stabilityTime)
        {
            stable = true;
        }
    }

    // -------------------------------------------------
    // Weight moved significantly
    // -------------------------------------------------

    else
    {
        stabilityStartTime = 0;
        stable = false;
    }

    // =================================================
    // SERIAL DEBUG
    // =================================================

    static unsigned long lastWeightDebug = 0;

    if (millis() -
        lastWeightDebug >= 500)
    {
        lastWeightDebug = millis();

        Serial.print("[WEIGHT] RAW: ");
        Serial.print(rawWeight, 2);

        Serial.print(" g | FILTERED: ");
        Serial.print(filteredWeight, 2);

        Serial.print(" g | STABLE: ");

        if (stable)
        {
            Serial.println("YES");
        }
        else
        {
            Serial.println("NO");
        }
    }
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

    Serial.println(
        "[WEIGHT] Starting tare..."
    );

    loadCell.tareNoDelay();

    taring = true;

    stable = false;

    stabilityStartTime = 0;
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
// GET FILTERED WEIGHT
// =========================================================

float WeightSensor::getWeight() const
{
    return filteredWeight;
}

// =========================================================
// GET RAW WEIGHT
// =========================================================

float WeightSensor::getRawWeight() const
{
    return rawWeight;
}

// =========================================================
// STABLE
// =========================================================

bool WeightSensor::isStable() const
{
    return stable;
}

// =========================================================
// READY
// =========================================================

bool WeightSensor::isReady() const
{
    return ready;
}

// =========================================================
// TARING
// =========================================================

bool WeightSensor::isTaring() const
{
    return taring;
}