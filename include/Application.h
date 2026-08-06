#ifndef APPLICATION_H
#define APPLICATION_H

#include "Display.h"

class Application
{
public:
    Application();

    void begin();
    void update();

private:
    Display display;
    unsigned long lastHeartbeat;
};

#endif