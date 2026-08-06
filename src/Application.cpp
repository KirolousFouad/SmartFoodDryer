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

    mainMenu.setTitle("Food Dryer");
    mainMenu.addItem("Auto");
    mainMenu.addItem("Manual");

    display.drawMenu(
    mainMenu.getTitle(),
    mainMenu.getItem(mainMenu.getSelectedIndex()));

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

    switch(event)
    {
        case ENCODER_LEFT:

            mainMenu.previous();
            display.drawMenu(
                mainMenu.getTitle(),
                mainMenu.getItem(mainMenu.getSelectedIndex())
            );
        break;

        case ENCODER_RIGHT:

            mainMenu.next();
            display.drawMenu(
            mainMenu.getTitle(),
            mainMenu.getItem(mainMenu.getSelectedIndex())
        );

        break;

        default:
        break;
    }
}