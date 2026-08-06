#include "Dryer.h"

#include "States.h"

State state;

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