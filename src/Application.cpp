#include <Arduino.h>
#include "Application.h"

Application::Application()
    : encoder(2, 3, 4)
{
    lastHeartbeat = 0;
}

void Application::begin()
{
    Serial.begin(115200);

    display.begin();
    encoder.begin();

    // Splash Screen
    display.center(0, "Smart Dryer");
    display.center(1, "Firmware v0.2");

    delay(3000);

    // Initialize Menus
    menuManager.begin();

    // Draw Main Menu
    display.drawMenu(
        menuManager.currentMenu()->getTitle(),
        menuManager.currentMenu()->getItem(
            menuManager.currentMenu()->getSelectedIndex()
        )
    );

    Serial.println();
    Serial.println("==============================");
    Serial.println(" Smart Food Dryer Firmware");
    Serial.println(" Version 0.2.0");
    Serial.println("==============================");
}

void Application::update()
{
    encoder.update();

    EncoderEvent event = encoder.getEvent();

    switch (event)
    {
    case ENCODER_LEFT:

        menuManager.currentMenu()->previous();

        display.drawMenu(
            menuManager.currentMenu()->getTitle(),
            menuManager.currentMenu()->getItem(
                menuManager.currentMenu()->getSelectedIndex()
            )
        );

        break;

    case ENCODER_RIGHT:

        menuManager.currentMenu()->next();

        display.drawMenu(
            menuManager.currentMenu()->getTitle(),
            menuManager.currentMenu()->getItem(
                menuManager.currentMenu()->getSelectedIndex()
            )
        );

        break;

    case ENCODER_CLICK:

    switch(menuManager.currentMenu()->getSelectedAction())
    {
        case ACTION_OPEN_RECIPES:

            menuManager.openRecipes();

            display.drawMenu(
                menuManager.currentMenu()->getTitle(),
                menuManager.currentMenu()->getItem(
                    menuManager.currentMenu()->getSelectedIndex()
                )
            );

        break;

        case ACTION_OPEN_MANUAL:

            Serial.println("Manual Mode");

        break;

        case ACTION_START_RECIPE:

            Serial.print("Starting Recipe ID: ");

            Serial.println(
                menuManager.currentMenu()->getSelectedParameter()
            );

        break;

        default:

        break;
    }

    break;  

    case ENCODER_LONG_CLICK:

    menuManager.openMain();

    display.drawMenu(
        menuManager.currentMenu()->getTitle(),
        menuManager.currentMenu()->getItem(
            menuManager.currentMenu()->getSelectedIndex()
        )
    );

    break;
    default:
        break;
}
}