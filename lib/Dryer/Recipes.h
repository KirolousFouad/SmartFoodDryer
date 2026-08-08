#ifndef RECIPE_H
#define RECIPE_H

#include <Arduino.h>

struct Recipe
{
    const char* name;

    uint8_t id;

    uint8_t temperature;

    uint16_t targetWeight;
};

#endif