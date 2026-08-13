#ifndef FAN_H
#define FAN_H

#include <Arduino.h>

class Fan
{
public:

    Fan(
        uint8_t circulationPin,
        uint8_t coolingPwmPin
    );

    void begin();

    // Fan 1: 2-wire circulation fan
    void circulationOn();
    void circulationOff();

    // Fan 2: 4-wire PWM cooling fan
    void setCoolingSpeed(uint8_t percent);
    void coolingOff();

    // Get current cooling speed
    uint8_t getCoolingSpeed() const;

private:

    uint8_t circulationPin;
    uint8_t coolingPwmPin;

    uint8_t coolingSpeed;
};

#endif