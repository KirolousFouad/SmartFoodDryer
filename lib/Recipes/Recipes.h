#ifndef RECIPES_H
#define RECIPES_H

#include <Arduino.h>

struct Recipe
{
    const char *name;
    byte temperature;
    byte finalPercent;
};

extern Recipe recipes[];

extern const byte recipeCount;

#endif