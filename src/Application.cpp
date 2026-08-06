#include <Arduino.h>
#include "Application.h"

Application::Application()
    : encoder(2,3,4)
{
    lastHeartbeat = 0;
}

void Application::begin()
{
    display.begin();
    encoder.begin();
    display.center(0, "Smart Dryer");
    display.center(1, "Firmware v0.2");
    delay(3000);
    display.clear();
    display.center(0,"System");
    display.center(1,"Ready");
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
    encoder.update();

    EncoderEvent event = encoder.getEvent();

    switch (event)
    {
    case ENCODER_LEFT:
        Serial.println("LEFT");
        break;

    case ENCODER_RIGHT:
        Serial.println("RIGHT");
        break;

    case ENCODER_CLICK:
        Serial.println("CLICK");
        break;

    case ENCODER_LONG_CLICK:
        Serial.println("LONG CLICK");
        break;

    default:
        break;
    }
}