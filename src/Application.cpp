#include <Arduino.h>
#include "Application.h"

Application::Application()
{
    lastHeartbeat = 0;
}

void Application::begin()
{
    Serial.begin(115200);

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