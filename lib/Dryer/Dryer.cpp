#include "Dryer.h"

// =====================================================
// CONSTRUCTOR
// =====================================================

Dryer::Dryer(
    uint8_t circulationFanPin,
    uint8_t coolingFanPwmPin
)
: running(false),
  fan(circulationFanPin, coolingFanPwmPin),
  heater(),
  heaterPower(0),
  targetTemperature(0),
  heaterWindowStart(0),
  heaterOutputState(false)
{
}

// =====================================================
// BEGIN
// =====================================================

void Dryer::begin()
{
    running = false;

    heaterPower = 0;
    targetTemperature = 0;

    heaterWindowStart = millis();
    heaterOutputState = false;

    fan.begin();
    heater.begin();
}

// =====================================================
// DRYER START
// =====================================================

void Dryer::start(uint8_t targetTemperatureValue)
{
    if (running)
        return;

    running = true;

    targetTemperature =
        targetTemperatureValue;

    heaterPower = 100;

    heaterWindowStart = millis();

    heaterOutputState = false;

    Serial.println();
    Serial.println("==============================");
    Serial.println(" DRYER START");
    Serial.println("==============================");

    Serial.print("Target Temperature: ");
    Serial.print(targetTemperature);
    Serial.println(" C");

    // =================================================
    // CIRCULATION FAN
    // Always ON during drying
    // =================================================

    fan.circulationOn();

    // =================================================
    // COOLING FAN
    // OFF during normal drying
    // =================================================

    fan.coolingOff();

    // =================================================
    // HEATER
    // =================================================

    heater.setTemperature(targetTemperature);

    // Start heater control
    heater.start();

    heaterOutputState = true;
}

// =====================================================
// DRYER STOP
// =====================================================

void Dryer::stop()
{
    if (!running)
        return;

    // Stop heater
    heater.stop();

    // Turn fans OFF
    fan.circulationOff();
    fan.coolingOff();

    running = false;

    heaterPower = 0;
    heaterOutputState = false;

    Serial.println();
    Serial.println("==============================");
    Serial.println(" DRYER STOP");
    Serial.println("==============================");
}

// =====================================================
// DRYER UPDATE
// =====================================================

void Dryer::update()
{
    if (!running)
        return;

    updateHeaterControl();
}

// =====================================================
// HEATER ON
// =====================================================

void Dryer::heaterOn()
{
    if (!running)
        return;

    heater.start();
}


// =====================================================
// HEATER OFF
// =====================================================

void Dryer::heaterOff()
{
    heater.stop();
}

// =====================================================
// HEATER POWER
// =====================================================

void Dryer::setHeaterPower(uint8_t percent)
{
    if (percent > 100)
        percent = 100;

    // Avoid unnecessary changes
    if (heaterPower == percent)
        return;

    heaterPower = percent;

    Serial.print("[HEATER] Power: ");
    Serial.print(heaterPower);
    Serial.println("%");

    // If zero, turn heater OFF immediately
    if (heaterPower == 0)
    {
        if (heaterOutputState)
        {
            heater.stop();
            heaterOutputState = false;
        }

        return;
    }

    // If dryer is running and power was changed
    // restart the timing window.
    heaterWindowStart = millis();
}

// =====================================================
// HEATER CONTROL
// =====================================================

void Dryer::updateHeaterControl()
{
    unsigned long now = millis();

    // Start a new control window
    if (now - heaterWindowStart >= HEATER_WINDOW)
    {
        heaterWindowStart = now;
    }

    // Calculate elapsed time inside current window
    unsigned long elapsed =
        now - heaterWindowStart;

    // Calculate ON time
    unsigned long onTime =
        (HEATER_WINDOW * heaterPower) / 100;

    bool shouldBeOn =
        elapsed < onTime;

    // -------------------------------------------------
    // Heater state changed
    // -------------------------------------------------

    if (shouldBeOn && !heaterOutputState)
    {
        heater.start();

        heaterOutputState = true;
    }

    else if (!shouldBeOn && heaterOutputState)
    {
        heater.stop();

        heaterOutputState = false;
    }
}

// =====================================================
// GET HEATER POWER
// =====================================================

uint8_t Dryer::getHeaterPower() const
{
    return heaterPower;
}

// =====================================================
// STATUS
// =====================================================

bool Dryer::isRunning() const
{
    return running;
}

// =====================================================
// CIRCULATION FAN
// =====================================================

void Dryer::circulationOn()
{
    fan.circulationOn();
}

void Dryer::circulationOff()
{
    fan.circulationOff();
}

// =====================================================
// COOLING FAN
// =====================================================

void Dryer::setCoolingSpeed(uint8_t percent)
{
    fan.setCoolingSpeed(percent);
}

void Dryer::coolingOff()
{
    fan.coolingOff();
}

uint8_t Dryer::getCoolingSpeed() const
{
    return fan.getCoolingSpeed();
}