#include "WeightSensor.h"

WeightSensor::WeightSensor(
    uint8_t dataPin,
    uint8_t clockPin
)
    : loadCell(dataPin, clockPin)
{
    weight = 0.0f;
    ready = false;
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

    loadCell.begin();

    // Allow HX711/load cell to stabilize
    delay(1000);

    // Tare
    loadCell.start(2000, true);

    if (loadCell.getTareTimeoutFlag())
    {
        Serial.println("ERROR: HX711 tare timeout");
        ready = false;
        return;
    }

    if (loadCell.getSignalTimeoutFlag())
    {
        Serial.println("ERROR: HX711 signal timeout");
        ready = false;
        return;
    }

    ready = true;

    Serial.println("HX711 ready");
    Serial.println("Load cell tared");
    Serial.println("==============================");
}


// =========================================================
// UPDATE
// =========================================================

void WeightSensor::update()
{
    if (!ready)
        return;

    if (loadCell.update())
    {
        weight = loadCell.getData();

        Serial.print("Weight: ");
        Serial.print(weight, 2);
        Serial.println(" g");
    }
}


// =========================================================
// TARE
// =========================================================

void WeightSensor::tare()
{
    if (!ready)
        return;

    Serial.println("Taring load cell...");

    loadCell.tareNoDelay();
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