#ifndef APPLICATION_H
#define APPLICATION_H

#include "Display.h"
#include "Encoder.h"
#include "Menu.h"
#include "MenuManager.h"

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
    
    unsigned long lastHeartbeat;
};

#endif