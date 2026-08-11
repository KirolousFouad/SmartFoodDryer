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

    delay(120);

    // Release
    digitalWrite(pin, HIGH);

    // Give SCR time to register one physical press
    delay(120);
}

// =====================================================
// INCREASE ONE STEP
// =====================================================

void SCRController::increase()
{
    if (currentPower >= MAX_POWER)
        return;

    pressButton(increasePin);

    // Exactly ONE physical pulse = ONE software %
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
        return;

    pressButton(decreasePin);

    // Exactly ONE physical pulse = ONE software %
    currentPower--;

    Serial.print("[SCR] Power decreased to ");
    Serial.print(currentPower);
    Serial.println("%");
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

    if (percent == currentPower)
        return;

    Serial.print("[SCR] Setting power from ");
    Serial.print(currentPower);
    Serial.print("% to ");
    Serial.print(percent);
    Serial.println("%");

    // Increase one physical step at a time
    while (currentPower < percent)
    {
        increase();
    }

    // Decrease one physical step at a time
    while (currentPower > percent)
    {
        decrease();
    }
}

// =====================================================
// RESET SCR TO ZERO
// =====================================================
void SCRController::resetToZero()
{
    Serial.println("[SCR] Forcing physical SCR to 0%...");

    // Send 100 physical decrease pulses.
    // The SCR will stop at 0%.
    for (uint8_t i = 0; i < 100; i++)
    {
        pressButton(decreasePin);
    }

    currentPower = 0;

    Serial.println("[SCR] Physical SCR reset complete: 0%");
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

    // Already there
    if (currentPower == targetPower)
        return true;

    // ONE physical pulse only
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