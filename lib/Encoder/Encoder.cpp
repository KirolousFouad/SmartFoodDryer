#include "Encoder.h"

Encoder::Encoder(uint8_t clk,
                 uint8_t dt,
                 uint8_t sw)
{
    clkPin = clk;
    dtPin = dt;
    swPin = sw;

    event = ENCODER_NONE;
}

void Encoder::begin()
{
    pinMode(clkPin, INPUT_PULLUP);
    pinMode(dtPin, INPUT_PULLUP);
    pinMode(swPin, INPUT_PULLUP);

    lastCLK = digitalRead(clkPin);
    lastButton = digitalRead(swPin);

    pressTime = 0;
}

void Encoder::update()
{
    event = ENCODER_NONE;

    bool currentCLK = digitalRead(clkPin);

    // Detect only the falling edge of CLK
    if (lastCLK == HIGH && currentCLK == LOW)
    {
        if (digitalRead(dtPin) == HIGH)
            event = ENCODER_RIGHT;
        else
            event = ENCODER_LEFT;
    }

    lastCLK = currentCLK;

    bool currentButton = digitalRead(swPin);

    if (!currentButton && lastButton)
    {
        pressTime = millis();
    }

    if (currentButton && !lastButton)
    {
        if (millis() - pressTime > 1000)
            event = ENCODER_LONG_CLICK;
        else
            event = ENCODER_CLICK;
    }

    lastButton = currentButton;
}

EncoderEvent Encoder::getEvent()
{
    EncoderEvent e = event;

    event = ENCODER_NONE;

    return e;
}