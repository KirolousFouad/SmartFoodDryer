#ifndef APPLICATION_H
#define APPLICATION_H

#include "Display.h"
#include "Encoder.h"
#include "Menu.h"

class Application
{
public:
    Application();

    void begin();
    void update();

private:
    Display display;
    Encoder encoder;
    Menu mainMenu;
    
    unsigned long lastHeartbeat;
};

#endif