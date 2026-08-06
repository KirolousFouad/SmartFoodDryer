#ifndef DRYER_H
#define DRYER_H

#include <Arduino.h>

enum Screen
{
    MAIN_MENU,
    AUTO_MENU,
    RECIPE_INFO,
    MANUAL_TEMP,
    MANUAL_WEIGHT,
    READY,
    DRYING
};

class Dryer
{
public:
    Screen screen;
    bool autoMode;
    byte selectedRecipe;
    int targetTemp;
    int targetWeight;

    Dryer();
    void reset();
};

extern Dryer dryer;

#endif