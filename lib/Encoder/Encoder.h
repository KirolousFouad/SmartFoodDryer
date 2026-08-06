#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

class Encoder
{
private:

    int lastCLK;

    bool pressedLast;

public:

    void begin();

    int getRotation();

    bool isPressed();
};

extern Encoder encoder;

#endif