#include "Dryer.h"

#include "SystemState.h"

SystemState state;
struct DryerSettings
{
    uint8_t targetTemperature;
    uint16_t targetWeight;
};

Dryer::Dryer()
{
    reset();
}

void Dryer::reset()
{
    state = STATE_MENU;

    autoMode = true;

    selectedRecipe = 0;

    targetTemp = 60;

    targetWeight = 1000;
}