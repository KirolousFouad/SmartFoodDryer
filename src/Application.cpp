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
    settings.targetWeight = 1000;
    settings.autoMode = false;
    settings.recipeID = 0;

    dryingStartTime = 0;
    lastDryerUpdate = 0;

    simulatedWeight = 1000;
    initialWeight = 1000;

    lastWeightUpdate = 0;
    finishScreenShown = false;
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

        initialWeight = 1000;
        simulatedWeight = initialWeight;

        dryingStartTime = millis();
        lastDryerUpdate = millis();
        lastWeightUpdate = millis();

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

    // --------------------------------
    // Calculate elapsed time
    // --------------------------------

    unsigned long elapsedSeconds =
        elapsed / 1000;

    unsigned int minutes =
        elapsedSeconds / 60;

    unsigned int seconds =
        elapsedSeconds % 60;

    // --------------------------------
    // Simulate drying weight loss
    // --------------------------------

    if (simulatedWeight > settings.targetWeight)
    {
        if (simulatedWeight - settings.targetWeight >= 5)
        {
            simulatedWeight -= 5;
        }
        else
        {
            // Prevent weight from going below target
            simulatedWeight = settings.targetWeight;
        }
    }

    // --------------------------------
    // Check if target weight is reached
    // --------------------------------

    if (simulatedWeight <= settings.targetWeight)
    {
        simulatedWeight = settings.targetWeight;

        state = STATE_FINISHED;

        Serial.println();
        Serial.println("==============================");
        Serial.println(" DRYING COMPLETE");
        Serial.print(" Final Weight: ");
        Serial.print(simulatedWeight);
        Serial.println(" g");
        Serial.println("==============================");
    }

    // --------------------------------
    // Calculate estimated remaining time
    // --------------------------------

    unsigned long estimatedRemainingSeconds = 0;

    unsigned int weightLost =
        initialWeight - simulatedWeight;

    if (weightLost > 0 &&
        simulatedWeight > settings.targetWeight)
    {
        unsigned int remainingWeight =
            simulatedWeight - settings.targetWeight;

        if (elapsedSeconds > 0)
        {
            // Weight loss rate in grams/second
            unsigned long rate =
                weightLost / elapsedSeconds;

            if (rate > 0)
            {
                estimatedRemainingSeconds =
                    remainingWeight / rate;
            }
        }
    }

    // --------------------------------
    // Convert remaining time
    // --------------------------------

    unsigned int remainingMinutes =
        estimatedRemainingSeconds / 60;

    unsigned int remainingSeconds =
        estimatedRemainingSeconds % 60;

    // --------------------------------
    // LCD Display
    // --------------------------------

    char line1[17];
    char line2[17];

    sprintf(
        line1,
        "T:%3dC W:%4dg",
        settings.temperature,
        simulatedWeight
    );

    sprintf(
        line2,
        "Remain:%02u:%02u",
        remainingMinutes,
        remainingSeconds
    );

    display.print(0, 0, line1);
    display.print(0, 1, line2);

    // --------------------------------
    // Serial Monitor
    // --------------------------------

    Serial.print("Drying | Temp: ");
    Serial.print(settings.temperature);

    Serial.print(" C | Weight: ");
    Serial.print(simulatedWeight);

    Serial.print(" g | Time: ");

    if (minutes < 10)
        Serial.print("0");

    Serial.print(minutes);
    Serial.print(":");

    if (seconds < 10)
        Serial.print("0");

    Serial.print(seconds);

    Serial.print(" | Remaining: ");

    if (remainingMinutes < 10)
        Serial.print("0");

    Serial.print(remainingMinutes);
    Serial.print(":");

    if (remainingSeconds < 10)
        Serial.print("0");

    Serial.println(remainingSeconds);
    
}
if (state == STATE_FINISHED)
{
    display.clear();

    display.center(0, "Drying");
    display.center(1, "Complete!");

    delay(2000);

    display.clear();

    char finalWeight[17];

    sprintf(
        finalWeight,
        "Final:%4dg",
        simulatedWeight
    );

    display.center(0, "Finished");
    display.center(1, finalWeight);
}
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