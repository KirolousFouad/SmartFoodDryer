#ifndef APPLICATION_H
#define APPLICATION_H

#include "Display.h"
#include "Encoder.h"

class Application
{
public:
    Application();

    void begin();
    void update();

private:
    Display display;
    Encoder encoder;
    unsigned long lastHeartbeat;
};

#endif