#include "Encoder.h"
#include "Config.h"

Encoder encoder;

void Encoder::begin()
{
    pinMode(ENCODER_CLK,INPUT);
    pinMode(ENCODER_DT,INPUT);
    pinMode(ENCODER_SW,INPUT_PULLUP);

    lastCLK = digitalRead(ENCODER_CLK);

    pressedLast = false;
}

int Encoder::getRotation()
{
    int currentCLK = digitalRead(ENCODER_CLK);

    if(currentCLK != lastCLK && currentCLK == HIGH)
    {
        lastCLK = currentCLK;

        if(digitalRead(ENCODER_DT) != currentCLK)
            return 1;
        else
            return -1;
    }

    lastCLK = currentCLK;

    return 0;
}

bool Encoder::isPressed()
{
    bool current = !digitalRead(ENCODER_SW);

    if(current && !pressedLast)
    {
        pressedLast = true;
        return true;
    }

    if(!current)
        pressedLast = false;

    return false;
}