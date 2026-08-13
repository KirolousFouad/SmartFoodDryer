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

    // -----------------------------------------------------
    // Physical SCR control
    // -----------------------------------------------------

    // Increase SCR by exactly one physical step.
    void increase();

    // Decrease SCR by exactly one physical step.
    void decrease();

    // -----------------------------------------------------
    // Reset
    // -----------------------------------------------------

    // Send one physical decrease pulse and force
    // the software state to 0%.
    //
    // Used by Application during the non-blocking
    // startup/error/finish reset sequence.
    void resetStep();

    // Legacy blocking reset.
    // Keep available for compatibility, but Application
    // should use resetStep() instead.
    void resetToZero();

    // -----------------------------------------------------
    // Power
    // -----------------------------------------------------

    // Set requested power.
    //
    // This is a blocking helper and should NOT be used
    // by the main Application control loop.
    void setPower(uint8_t percent);

    // Current software-tracked SCR position.
    uint8_t getPower() const;

    // Move exactly one physical step toward target.
    //
    // Returns true when target has been reached.
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