#include <Arduino.h>
#include "Application.h"

Application::Application()
    : encoder(2, 3, 4)
{
    lastHeartbeat = 0;

    // System state
    state = STATE_MENU;

    // Default manual settings
    settings.temperature = 70;
    settings.targetWeight = 2000;
    settings.autoMode = false;
    settings.recipeID = 0;

    dryingStartTime = 0;
    lastDryerUpdate = 0;
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

    //------------------------------------------------------
    // MANUAL TEMPERATURE EDIT SCREEN
    //------------------------------------------------------

    if (state == STATE_MANUAL_TEMP)
    {
        switch (event)
        {
        case ENCODER_LEFT:

            if (settings.temperature > 30)
                settings.temperature--;

            break;

        case ENCODER_RIGHT:

            if (settings.temperature < 90)
                settings.temperature++;

            break;

        case ENCODER_CLICK:

            state = STATE_MANUAL_WEIGHT;

            display.clear();
            display.print(0, 0, "Target Weight");

            {
                char buffer[17];
                sprintf(buffer, "%4d g", settings.targetWeight);
                display.print(0, 1, buffer);
            }

            break;

        case ENCODER_LONG_CLICK:

            state = STATE_MENU;

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

        if (state == STATE_MANUAL_TEMP)
        {
            char buffer[17];

            sprintf(buffer, "Temp: %2d C", settings.temperature);

            display.print(0, 0, "Manual Temp");
            display.print(0, 1, buffer);
        }

        return;
    }

    //------------------------------------------------------
    // MENU NAVIGATION
    //------------------------------------------------------
    //------------------------------------------------------
// MANUAL WEIGHT EDIT SCREEN
//------------------------------------------------------

if (state == STATE_MANUAL_WEIGHT)
{
    switch (event)
    {
    case ENCODER_LEFT:

        if (settings.targetWeight >= 50)
            settings.targetWeight -= 50;

        break;

    case ENCODER_RIGHT:

        if (settings.targetWeight <= 9950)
            settings.targetWeight += 50;

        break;

    case ENCODER_CLICK:

        state = STATE_READY;

        display.clear();
        display.print(0, 0, "Ready?");

        display.print(0, 1, "Press = Start");

        break;

    case ENCODER_LONG_CLICK:

        state = STATE_MENU;

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

    // Refresh weight display
    if (state == STATE_MANUAL_WEIGHT)
    {
        char buffer[17];

        sprintf(
            buffer,
            "Weight:%4d g",
            settings.targetWeight
        );

        display.print(0, 0, "Target Weight");
        display.print(0, 1, buffer);
    }

    return;
    }
//------------------------------------------------------
// READY SCREEN
//------------------------------------------------------

if (state == STATE_READY)
{
    switch (event)
    {
    case ENCODER_CLICK:

        state = STATE_RUNNING;

        dryingStartTime = millis();
        lastDryerUpdate = millis();

        display.clear();
        display.print(0, 0, "Starting...");

        delay(1000);

        display.clear();

        break;

    case ENCODER_LONG_CLICK:

        state = STATE_MANUAL_WEIGHT;

        display.clear();
        display.print(0, 0, "Target Weight");

        {
            char buffer[17];

            sprintf(
                buffer,
                "Weight:%4d g",
                settings.targetWeight
            );

            display.print(0, 1, buffer);
        }

        break;

    default:
        break;
    }

    return;
}
//------------------------------------------------------
// RUNNING SCREEN
//------------------------------------------------------
if (state == STATE_RUNNING)
{
    unsigned long elapsed =
        millis() - dryingStartTime;

    if (elapsed - lastDryerUpdate >= 1000)
    {
        lastDryerUpdate = millis();

        unsigned long elapsedSeconds =
            elapsed / 1000;

        unsigned int minutes =
            elapsedSeconds / 60;

        unsigned int seconds =
            elapsedSeconds % 60;

        char line1[17];
        char line2[17];

        sprintf(
            line1,
            "Temp:%3d C",
            settings.temperature
        );

        sprintf(
            line2,
            "Time:%02d:%02d",
            minutes,
            seconds
        );

        display.print(0, 0, line1);
        display.print(0, 1, line2);

        Serial.print("Drying | Temp: ");
        Serial.print(settings.temperature);
        Serial.print(" C | Time: ");
        Serial.print(minutes);
        Serial.print(":");

        if (seconds < 10)
            Serial.print("0");

        Serial.println(seconds);
    }

    if (event == ENCODER_LONG_CLICK)
    {
        state = STATE_MENU;

        menuManager.openMain();

        display.drawMenu(
            menuManager.currentMenu()->getTitle(),
            menuManager.currentMenu()->getItem(
                menuManager.currentMenu()->getSelectedIndex()
            )
        );
    }

    return;
}
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

        switch (menuManager.currentMenu()->getSelectedAction())
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

            state = STATE_MANUAL_TEMP;

            display.clear();
            display.print(0, 0, "Manual Temp");

            {
                char buffer[17];
                sprintf(buffer, "Temp: %2d C", settings.temperature);
                display.print(0, 1, buffer);
            }

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