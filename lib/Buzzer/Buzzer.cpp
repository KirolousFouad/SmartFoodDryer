#include "Buzzer.h"

// =====================================================
// CONSTRUCTOR
// =====================================================

Buzzer::Buzzer(uint8_t pin)
    : pin(pin),
      active(false),
      pattern(BUZZER_NONE),
      step(0),
      lastStepTime(0)
{
}

// =====================================================
// BEGIN
// =====================================================

void Buzzer::begin()
{
    pinMode(pin, OUTPUT);

    digitalWrite(
        pin,
        LOW
    );

    active = false;
    pattern = BUZZER_NONE;
    step = 0;
    lastStepTime = 0;
}

// =====================================================
// UPDATE
// =====================================================

void Buzzer::update()
{
    if (!active)
    {
        return;
    }

    unsigned long now = millis();

    switch (pattern)
    {
        // =================================================
        // SHORT BEEP
        // =================================================

        case BUZZER_SHORT:
        {
            if (step == 0)
            {
                tone(
                    pin,
                    2000
                );

                step = 1;
                lastStepTime = now;
            }
            else if (now - lastStepTime >= 100)
            {
                stop();

                return;
            }

            break;
        }

        // =================================================
        // SUCCESS
        // =================================================

        case BUZZER_SUCCESS:
        {
            if (step == 0)
            {
                tone(
                    pin,
                    2000
                );

                step = 1;
                lastStepTime = now;
            }
            else if (step == 1 &&
                     now - lastStepTime >= 150)
            {
                noTone(pin);

                step = 2;
                lastStepTime = now;
            }
            else if (step == 2 &&
                     now - lastStepTime >= 100)
            {
                tone(
                    pin,
                    2500
                );

                step = 3;
                lastStepTime = now;
            }
            else if (step == 3 &&
                     now - lastStepTime >= 150)
            {
                stop();

                return;
            }

            break;
        }

        // =================================================
        // WARNING
        // =================================================

        case BUZZER_WARNING:
        {
            if (step == 0)
            {
                tone(
                    pin,
                    1500
                );

                step = 1;
                lastStepTime = now;
            }
            else if (step == 1 &&
                     now - lastStepTime >= 300)
            {
                stop();

                return;
            }

            break;
        }

        // =================================================
        // ERROR
        // =================================================

        case BUZZER_ERROR:
        {
            /*
             * Repeating:
             *
             * ON 500 ms
             * OFF 300 ms
             */

            if (step == 0)
            {
                tone(
                    pin,
                    1000
                );

                step = 1;
                lastStepTime = now;
            }
            else if (step == 1 &&
                     now - lastStepTime >= 500)
            {
                noTone(pin);

                step = 2;
                lastStepTime = now;
            }
            else if (step == 2 &&
                     now - lastStepTime >= 300)
            {
                step = 0;
                lastStepTime = now;
            }

            break;
        }

        default:
        {
            stop();

            break;
        }
    }
}

// =====================================================
// SHORT BEEP
// =====================================================

void Buzzer::beepShort()
{
    startPattern(
        BUZZER_SHORT
    );
}

// =====================================================
// LONG BEEP
// =====================================================

void Buzzer::beepLong()
{
    startPattern(
        BUZZER_WARNING
    );
}

// =====================================================
// SUCCESS
// =====================================================

void Buzzer::success()
{
    startPattern(
        BUZZER_SUCCESS
    );
}

// =====================================================
// WARNING
// =====================================================

void Buzzer::warning()
{
    startPattern(
        BUZZER_WARNING
    );
}

// =====================================================
// ERROR
// =====================================================

void Buzzer::error()
{
    startPattern(
        BUZZER_ERROR
    );
}

// =====================================================
// START PATTERN
// =====================================================

void Buzzer::startPattern(
    BuzzerPattern newPattern
)
{
    noTone(pin);

    pattern = newPattern;

    step = 0;

    lastStepTime = millis();

    active = true;
}

// =====================================================
// STOP
// =====================================================

void Buzzer::stop()
{
    noTone(pin);

    digitalWrite(
        pin,
        LOW
    );

    active = false;

    pattern = BUZZER_NONE;

    step = 0;
}