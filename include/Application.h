#ifndef APPLICATION_H
#define APPLICATION_H

#include "Display.h"
#include "Encoder.h"
#include "Menu.h"
#include "MenuManager.h"
#include "SystemState.h"
#include "DryerSettings.h"

class Application
{
public:
    Application();

    void begin();
    void update();

private:
    Display display;
    Encoder encoder;
    MenuManager menuManager;
    SystemState state;
    DryerSettings settings;
    
    unsigned long lastHeartbeat;
    
    unsigned long dryingStartTime;
    unsigned long lastDryerUpdate;

    unsigned int simulatedWeight;
    unsigned int initialWeight;
    unsigned long lastWeightUpdate;
    bool finishScreenShown;
};
#endif