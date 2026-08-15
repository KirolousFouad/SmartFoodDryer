#include "SCRController.h"

// =====================================================
// CONSTRUCTOR
// =====================================================

SCRController::SCRController(
    uint8_t increasePin,
    uint8_t decreasePin
)
    : increasePin(increasePin),
      decreasePin(decreasePin),
      currentPower(0)
{
}

// =====================================================
// BEGIN
// =====================================================

void SCRController::begin()
{
    pinMode(increasePin, OUTPUT);
    pinMode(decreasePin, OUTPUT);

    // Buttons released
    digitalWrite(increasePin, HIGH);
    digitalWrite(decreasePin, HIGH);

    currentPower = 0;

    Serial.println("[INIT] SCR Controller...");
    Serial.println("[SCR] Current Power: 0%");
    Serial.println("[INIT] SCR Controller OK");
}

// =====================================================
// PHYSICAL BUTTON PULSE
// =====================================================

void SCRController::pressButton(uint8_t pin)
{
    // Press
    digitalWrite(pin, LOW);

    delay(60);

    // Release
    digitalWrite(pin, HIGH);

    // Give SCR time to register the physical press
    delay(60);
}

// =====================================================
// INCREASE ONE STEP
// =====================================================

void SCRController::increase()
{
    if (currentPower >= MAX_POWER)
    {
        return;
    }

    pressButton(increasePin);

    currentPower++;

    Serial.print("[SCR] Power increased to ");
    Serial.print(currentPower);
    Serial.println("%");
}

// =====================================================
// DECREASE ONE STEP
// =====================================================

void SCRController::decrease()
{
    if (currentPower <= MIN_POWER)
    {
        return;
    }

    pressButton(decreasePin);

    currentPower--;

    Serial.print("[SCR] Power decreased to ");
    Serial.print(currentPower);
    Serial.println("%");
}

// =====================================================
// RESET ONE PHYSICAL STEP
// =====================================================

void SCRController::resetStep()
{
    /*
     * This method is intentionally different from decrease().
     *
     * During a physical reset we do NOT trust the software
     * power value because the actual SCR position may be
     * unknown after power-up or after an abnormal condition.
     *
     * Therefore:
     *
     * 1. Always send one physical DECREASE pulse.
     * 2. Force the software state to 0%.
     *
     * Application can call this repeatedly.
     *
     * This guarantees that enough physical decrease pulses
     * can be sent even after currentPower has already become 0.
     */

    pressButton(decreasePin);

    currentPower = 0;

    Serial.println(
        "[SCR] Reset pulse sent -> software power 0%"
    );
}

// =====================================================
// SET POWER
// =====================================================
void SCRController::setPower(uint8_t percent)
{
    percent = constrain(
        percent,
        MIN_POWER,
        MAX_POWER
    );

    moveOneStepToward(percent);
}

// =====================================================
// RESET SCR TO ZERO
// =====================================================

void SCRController::resetToZero()
{
    Serial.println(
        "[SCR] Forcing physical SCR to 0%..."
    );

    /*
     * The physical SCR position may be unknown.
     *
     * Therefore send 100 physical decrease pulses.
     *
     * This function is intentionally blocking.
     *
     * The Application should use resetStep() instead
     * during normal operation.
     */

    for (uint8_t i = 0; i < 100; i++)
    {
        pressButton(decreasePin);
    }

    currentPower = 0;

    Serial.println(
        "[SCR] Physical SCR reset complete: 0%"
    );
}

// =====================================================
// GET POWER
// =====================================================

uint8_t SCRController::getPower() const
{
    return currentPower;
}

// =====================================================
// MOVE ONE STEP TOWARD TARGET
// =====================================================
bool SCRController::moveOneStepToward(uint8_t targetPower)
{
    targetPower = constrain(
        targetPower,
        MIN_POWER,
        MAX_POWER
    );

    if (currentPower == targetPower)
    {
        return true;
    }

    if (currentPower < targetPower)
    {
        increase();
    }
    else
    {
        decrease();
    }

    return (currentPower == targetPower);
}