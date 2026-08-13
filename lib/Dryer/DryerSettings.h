#ifndef DRYER_SETTINGS_H
#define DRYER_SETTINGS_H

#include <Arduino.h>

struct DryerSettings
{
    uint8_t temperature;

    uint16_t targetWeight;

    bool autoMode;

    uint8_t recipeID;
};

#endif