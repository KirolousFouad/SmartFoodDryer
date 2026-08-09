#ifndef DRYER_H
#define DRYER_H

#include <Arduino.h>

#include "Fan.h"
#include "Heater.h"

class Dryer
{
public:

    Dryer(
        uint8_t circulationFanPin,
        uint8_t coolingFanPwmPin
    );

    void begin();

    void start(uint8_t targetTemperature);
    void stop();

    void update();

    bool isRunning() const;

    // =====================================================
    // Fan control
    // =====================================================

    void circulationOn();
    void circulationOff();

    void setCoolingSpeed(uint8_t percent);
    void coolingOff();

    uint8_t getCoolingSpeed() const;

private:

    bool running;

    Fan fan;
    Heater heater;
};

#endif