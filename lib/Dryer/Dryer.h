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
        uint8_t coolingFanPwmPin);

    void begin();

    // Start / stop dryer
    void start(uint8_t targetTemperature);
    void stop();

    // Called continuously from Application
    void update();

    // Heater proportional control
    void setHeaterPower(uint8_t percent);
    uint8_t getHeaterPower() const;

    // Status
    bool isRunning() const;

    // Circulation fan
    void circulationOn();
    void circulationOff();

    // Cooling fan
    void setCoolingSpeed(uint8_t percent);
    void coolingOff();
    uint8_t getCoolingSpeed() const;
    void heaterOn();
    void heaterOff();

private:
    bool running;

    Fan fan;
    Heater heater;

    // Heater control
    uint8_t heaterPower;
    uint8_t targetTemperature;

    // Time-proportional control window
    unsigned long heaterWindowStart;

    static const unsigned long HEATER_WINDOW =
        2000;

    bool heaterOutputState;

    void updateHeaterControl();
};

#endif