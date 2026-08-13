#ifndef SCR_CONTROLLER_H
#define SCR_CONTROLLER_H

#include <Arduino.h>

class SCRController
{
public:

    SCRController(
        uint8_t increasePin,
        uint8_t decreasePin
    );

    void begin();

    // Increase SCR by exactly 1 physical step
    void increase();

    // Decrease SCR by exactly 1 physical step
    void decrease();

    // Set requested power
    void setPower(uint8_t percent);

    // Force SCR back to 0%
    void resetToZero();

    // Current software-tracked SCR percentage
    uint8_t getPower() const;
    bool moveOneStepToward(uint8_t targetPower);

private:

    uint8_t increasePin;
    uint8_t decreasePin;

    uint8_t currentPower;

    static constexpr uint8_t MIN_POWER = 0;
    static constexpr uint8_t MAX_POWER = 100;

    void pressButton(uint8_t pin);
};

#endif