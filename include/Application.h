#ifndef APPLICATION_H
#define APPLICATION_H

class Application
{
public:
    Application();

    void begin();
    void update();

private:
    unsigned long lastHeartbeat;
};

#endif