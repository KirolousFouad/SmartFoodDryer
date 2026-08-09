#include "Dryer.h"

Dryer::Dryer(
    uint8_t circulationFanPin,
    uint8_t coolingFanPwmPin
)
    : running(false),
      fan(circulationFanPin, coolingFanPwmPin),
      heater()
{
}

void Dryer::begin()
{
    running = false;

    fan.begin();
    heater.begin();
}

// =====================================================
// DRYER START
// =====================================================

void Dryer::start(uint8_t targetTemperature)
{
    if (running)
        return;

    running = true;

    Serial.println();
    Serial.println("==============================");
    Serial.println(" DRYER START");
    Serial.println("==============================");

    Serial.print("Target Temperature: ");
    Serial.print(targetTemperature);
    Serial.println(" C");

    // Circulation fan always runs at 100%
    fan.circulationOn();

    // Cooling fan initially OFF
    fan.coolingOff();

    // Set and start heater
    heater.setTemperature(targetTemperature);
    heater.start();
}

// =====================================================
// DRYER STOP
// =====================================================

void Dryer::stop()
{
    if (!running)
        return;

    heater.stop();

    // Turn both fans OFF
    fan.circulationOff();
    fan.coolingOff();

    running = false;

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

    // Temperature-control logic will be added later.
}

// =====================================================
// STATUS
// =====================================================

bool Dryer::isRunning() const
{
    return running;
}

// =====================================================
// FAN CONTROL
// =====================================================

void Dryer::circulationOn()
{
    fan.circulationOn();
}

void Dryer::circulationOff()
{
    fan.circulationOff();
}

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