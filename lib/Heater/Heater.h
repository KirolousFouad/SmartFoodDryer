#ifndef HEATER_H
#define HEATER_H

#include <Arduino.h>

class Heater
{
public:

    Heater();

    void begin();

    void setTemperature(uint8_t temperature);

    void start();

    void stop();

    bool isRunning() const;

    uint8_t getTargetTemperature() const;

private:

    bool running;
    uint8_t targetTemperature;
};

#endif