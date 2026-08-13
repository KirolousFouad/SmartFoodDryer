#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

enum EncoderEvent
{
    ENCODER_NONE = 0,
    ENCODER_LEFT,
    ENCODER_RIGHT,
    ENCODER_CLICK,
    ENCODER_LONG_CLICK
};

class Encoder
{
public:
    Encoder(uint8_t clkPin,
            uint8_t dtPin,
            uint8_t swPin);

    void begin();
    void update();

    EncoderEvent getEvent();

private:
    uint8_t clkPin;
    uint8_t dtPin;
    uint8_t swPin;

    bool lastCLK;
    bool lastButton;

    unsigned long pressTime;

    EncoderEvent event;
};

#endif