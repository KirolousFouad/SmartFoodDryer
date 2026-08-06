#include <Arduino.h>
#include "Application.h"

Application::Application()
{
    lastHeartbeat = 0;
}

void Application::begin()
{
    Serial.begin(115200);
    display.begin();
    display.center(0, "Smart Dryer");
    display.center(1, "Firmware v0.2");
    delay(3000);
    display.clear();
    display.center(0,"System");
    display.center(1,"Ready");
    while (!Serial)
    {
        // Wait for Serial on supported boards
    }

    Serial.println();
    Serial.println("==================================");
    Serial.println(" Smart Food Dryer Firmware");
    Serial.println(" Version 0.2.0");
    Serial.println("==================================");
}

void Application::update()
{
    if (millis() - lastHeartbeat >= 1000)
    {
        lastHeartbeat = millis();

        Serial.println("Application Running...");
    }
}