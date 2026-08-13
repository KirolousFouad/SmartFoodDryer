#include "Fan.h"

Fan::Fan(
    uint8_t circulationPin,
    uint8_t coolingPwmPin
)
{
    this->circulationPin = circulationPin;
    this->coolingPwmPin = coolingPwmPin;

    coolingSpeed = 0;
}

void Fan::begin()
{
    pinMode(circulationPin, OUTPUT);
    pinMode(coolingPwmPin, OUTPUT);

    // Start safely with both fans OFF
    digitalWrite(circulationPin, LOW);

    analogWrite(coolingPwmPin, 0);

    coolingSpeed = 0;
}

// =====================================================
// FAN 1 — 2-WIRE CIRCULATION FAN
// =====================================================

void Fan::circulationOn()
{
    digitalWrite(circulationPin, HIGH);
}

void Fan::circulationOff()
{
    digitalWrite(circulationPin, LOW);
}

// =====================================================
// FAN 2 — 4-WIRE PWM COOLING FAN
// =====================================================

void Fan::setCoolingSpeed(uint8_t percent)
{
    // Protect against values above 100%
    if (percent > 100)
        percent = 100;

    coolingSpeed = percent;

    // Convert 0–100% to Arduino PWM 0–255
    uint8_t pwmValue =
        map(percent, 0, 100, 0, 255);

    analogWrite(
        coolingPwmPin,
        pwmValue
    );
}

void Fan::coolingOff()
{
    setCoolingSpeed(0);
}

uint8_t Fan::getCoolingSpeed() const
{
    return coolingSpeed;
}