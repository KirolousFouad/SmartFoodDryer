#include "Heater.h"

Heater::Heater()
{
    running = false;
    targetTemperature = 0;
}

void Heater::begin()
{
    running = false;
    targetTemperature = 0;
}

void Heater::setTemperature(uint8_t temperature)
{
    targetTemperature = temperature;

    Serial.print("Heater target temperature: ");
    Serial.print(targetTemperature);
    Serial.println(" C");
}

void Heater::start()
{
    running = true;

    Serial.println("Heater started");
}

void Heater::stop()
{
    running = false;

    Serial.println("Heater stopped");
}

bool Heater::isRunning() const
{
    return running;
}

uint8_t Heater::getTargetTemperature() const
{
    return targetTemperature;
}