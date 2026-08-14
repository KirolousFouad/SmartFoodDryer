#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

enum BuzzerPattern
{
    BUZZER_NONE,

    BUZZER_SHORT,

    BUZZER_SUCCESS,

    BUZZER_WARNING,

    BUZZER_ERROR
};

class Buzzer
{
public:

    explicit Buzzer(
        uint8_t pin
    );

    void begin();

    void update();

    void beepShort();

    void beepLong();

    void success();

    void warning();

    void error();

    void stop();

private:

    uint8_t pin;

    bool active;

    BuzzerPattern pattern;

    uint8_t step;

    unsigned long lastStepTime;

    void startPattern(
        BuzzerPattern pattern
    );
};

#endif