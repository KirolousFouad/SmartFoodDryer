#include "Application.h"
#include "Config.h"
#include "RecipeDatabase.h"

#include <Arduino.h>
#include <math.h>

// =====================================================
// CONSTRUCTOR
// =====================================================

Application::Application()
    : display(),
      encoder(
          ENCODER_CLK,
          ENCODER_DT,
          ENCODER_SW),
      menuManager(),
      state(STATE_MENU),
      settings(),
      dryer(
          8, // Circulation fan
          9  // Cooling fan PWM
          ),
      temperatureManager(
          5,  // DS18B20 data
          13, // MAX6675 SCK
          10, // MAX6675 CS
          12  // MAX6675 SO / DO
          ),
      lastHeartbeat(0),
      dryingStartTime(0),
      lastDryerUpdate(0),
      lastDisplayUpdate(0),
      lastTemperatureControl(0),
      heaterEnabled(false),
      finishScreenShown(false)
{
    settings.temperature = 60;
    settings.targetWeight = 0;
    settings.autoMode = false;
    settings.recipeID = 0;
}

// =====================================================
// BEGIN
// =====================================================

void Application::begin()
{
    Serial.begin(115200);

    delay(500);

    Serial.println();
    Serial.println("==============================");
    Serial.println(" SMART FOOD DRYER");
    Serial.println(" Firmware Version 0.2.0");
    Serial.println("==============================");

    // -------------------------------------------------
    // Display
    // -------------------------------------------------

    Serial.println("[INIT] Display...");

    display.begin();

    display.clear();

    display.center(
        0,
        "Smart Dryer"
    );

    display.center(
        1,
        "Starting..."
    );

    delay(1000);

    Serial.println("[INIT] Display OK");

    // -------------------------------------------------
    // Encoder
    // -------------------------------------------------

    Serial.println("[INIT] Encoder...");

    encoder.begin();

    Serial.println("[INIT] Encoder OK");

    // -------------------------------------------------
    // Menu Manager
    // -------------------------------------------------

    Serial.println("[INIT] Menu Manager...");

    menuManager.begin();

    Serial.println("[INIT] Menu Manager OK");

    // -------------------------------------------------
    // Dryer
    // -------------------------------------------------

    Serial.println("[INIT] Dryer...");

    dryer.begin();

    Serial.println("[INIT] Dryer OK");

    // -------------------------------------------------
    // Temperature Manager
    // -------------------------------------------------

    Serial.println("[INIT] Temperature Manager...");

    temperatureManager.begin();

    Serial.println("[INIT] Temperature Manager OK");

    // -------------------------------------------------
    // Initial State
    // -------------------------------------------------

    state = STATE_MENU;

    lastHeartbeat = millis();

    dryingStartTime = 0;

    lastDryerUpdate = millis();

    lastDisplayUpdate = millis();

    lastTemperatureControl = millis();
    heaterEnabled = false;

    finishScreenShown = false;

    menuManager.openMain();

    display.clear();

    drawCurrentMenu();

    Serial.println();
    Serial.println("==============================");
    Serial.println(" SYSTEM READY");
    Serial.println("==============================");
}

// =====================================================
// UPDATE
// =====================================================

void Application::update()
{
    // -------------------------------------------------
    // Encoder
    // -------------------------------------------------

    encoder.update();

    EncoderEvent event =
        encoder.getEvent();

    // -------------------------------------------------
    // Temperature update every 1 second
    // -------------------------------------------------

    static unsigned long lastTemperatureUpdate = 0;

    if (millis() - lastTemperatureUpdate >= 1000)
    {
        temperatureManager.update();

        lastTemperatureUpdate = millis();

        if (state == STATE_RUNNING)
        {
            updateTemperatureControl();
        }
    }

    // -------------------------------------------------
    // Dryer update
    // -------------------------------------------------

    if (millis() - lastDryerUpdate >= 50)
    {
        dryer.update();

        lastDryerUpdate =
            millis();
    }

    // -------------------------------------------------
    // Application state
    // -------------------------------------------------

    switch (state)
    {
        case STATE_MENU:

            handleMenu(event);

            break;

        case STATE_MANUAL_TEMP:

            handleManualTemperature(event);

            break;

        case STATE_MANUAL_WEIGHT:

            handleManualWeight(event);

            break;

        case STATE_READY:

            handleReady(event);

            break;

        case STATE_RUNNING:

            handleRunning(event);

            break;

        case STATE_PAUSED:

            handlePaused(event);

            break;

        case STATE_FINISHED:

            handleFinished(event);

            break;

        case STATE_ERROR:

            handleError(event);

            break;
    }

    // -------------------------------------------------
    // Heartbeat
    // -------------------------------------------------

    if (millis() - lastHeartbeat >= 5000)
    {
        Serial.println(
            "[Application] System alive"
        );

        lastHeartbeat = millis();
    }
}

// =====================================================
// TEMPERATURE CONTROL
// =====================================================
/*
void Application::updateTemperatureControl()
{
    float currentTemperature =
        temperatureManager.getAverageTemperature();

    float hotTemperature =
        temperatureManager.getHotTemperature();

    // -------------------------------------------------
    // Safety
    // -------------------------------------------------

    if (!isnan(hotTemperature) &&
        hotTemperature >= 90.0f)
    {
        Serial.println();
        Serial.println(
            "!!! SAFETY TEMPERATURE LIMIT !!!"
        );

        dryer.setHeaterPower(0);
        dryer.stop();

        state = STATE_ERROR;

        drawErrorScreen();

        return;
    }

    // -------------------------------------------------
    // Invalid chamber temperature
    // -------------------------------------------------

    if (isnan(currentTemperature))
    {
        Serial.println(
            "[CONTROL] Chamber temperature invalid"
        );

        dryer.setHeaterPower(0);

        return;
    }

    // -------------------------------------------------
    // Calculate heater power
    // -------------------------------------------------

    uint8_t power =
        calculateHeaterPower(
            currentTemperature,
            settings.temperature
        );

    dryer.setHeaterPower(power);
}
*/
// =====================================================
// CALCULATE HEATER POWER
// =====================================================

uint8_t Application::calculateHeaterPower(
    float currentTemperature,
    float targetTemperature
)
{
    float difference =
        targetTemperature - currentTemperature;

    // -------------------------------------------------
    // Already above target
    // -------------------------------------------------

    if (difference <= 0.0f)
    {
        return 0;
    }

    // -------------------------------------------------
    // 10°C or more below target
    // -------------------------------------------------

    if (difference >= 10.0f)
    {
        return 100;
    }

    // -------------------------------------------------
    // 5°C below target
    // -------------------------------------------------

    if (difference >= 5.0f)
    {
        return 70;
    }

    // -------------------------------------------------
    // 2°C below target
    // -------------------------------------------------

    if (difference >= 2.0f)
    {
        return 40;
    }

    // -------------------------------------------------
    // Less than 2°C below target
    // -------------------------------------------------

    return 20;
}

// =====================================================
// MENU HANDLING
// =====================================================

void Application::handleMenu(
    EncoderEvent event
)
{
    Menu* menu =
        menuManager.currentMenu();

    if (menu == nullptr)
        return;

    if (event == ENCODER_RIGHT)
    {
        menu->next();

        drawCurrentMenu();
    }

    else if (event == ENCODER_LEFT)
    {
        menu->previous();

        drawCurrentMenu();
    }

    else if (event == ENCODER_CLICK)
    {
        MenuAction action =
            menu->getSelectedAction();

        uint8_t parameter =
            menu->getSelectedParameter();

        handleMenuAction(
            action,
            parameter
        );
    }
}

// =====================================================
// MENU ACTIONS
// =====================================================

void Application::handleMenuAction(
    MenuAction action,
    uint8_t parameter
)
{
    switch (action)
    {
        case ACTION_OPEN_RECIPES:
        {
            settings.autoMode = true;

            menuManager.openRecipes();

            state = STATE_MENU;

            drawCurrentMenu();

            break;
        }

        case ACTION_OPEN_MANUAL:
        {
            settings.autoMode = false;

            settings.temperature = 60;
            settings.targetWeight = 1000;

            state = STATE_MANUAL_TEMP;

            drawManualTemperature();

            break;
        }

        case ACTION_START_RECIPE:
        {
            settings.recipeID =
                parameter;

            const Recipe& recipe =
                RecipeDatabase::getRecipe(
                    settings.recipeID
                );

            settings.temperature =
                recipe.temperature;

            settings.targetWeight =
                recipe.targetWeight;

            settings.autoMode = true;

            Serial.println();
            Serial.println(
                "=============================="
            );

            Serial.println(
                " RECIPE SELECTED"
            );

            Serial.println(
                "=============================="
            );

            Serial.print("Recipe: ");
            Serial.println(recipe.name);

            Serial.print("Temperature: ");
            Serial.print(recipe.temperature);
            Serial.println(" C");

            Serial.print("Target weight: ");
            Serial.print(recipe.targetWeight);
            Serial.println(" g");

            state = STATE_READY;

            drawReadyScreen();

            break;
        }

        case ACTION_CONFIRM_RECIPE:
        {
            state = STATE_READY;

            drawReadyScreen();

            break;
        }

        case ACTION_CANCEL_RECIPE:
        {
            menuManager.openRecipes();

            state = STATE_MENU;

            drawCurrentMenu();

            break;
        }

        case ACTION_NONE:
        default:
        {
            break;
        }
    }
}

// =====================================================
// DRAW CURRENT MENU
// =====================================================

void Application::drawCurrentMenu()
{
    Menu* menu =
        menuManager.currentMenu();

    if (menu == nullptr)
        return;

    display.drawMenu(
        menu->getTitle(),
        menu->getItem(
            menu->getSelectedIndex()
        )
    );
}

// =====================================================
// MANUAL TEMPERATURE
// =====================================================

void Application::handleManualTemperature(
    EncoderEvent event
)
{
    if (event == ENCODER_RIGHT)
    {
        if (settings.temperature < MAX_TEMP)
        {
            settings.temperature++;
        }

        drawManualTemperature();
    }

    else if (event == ENCODER_LEFT)
    {
        if (settings.temperature > MIN_TEMP)
        {
            settings.temperature--;
        }

        drawManualTemperature();
    }

    else if (event == ENCODER_CLICK)
    {
        state = STATE_MANUAL_WEIGHT;

        drawManualWeight();
    }

    else if (event == ENCODER_LONG_CLICK)
    {
        menuManager.openMain();

        state = STATE_MENU;

        drawCurrentMenu();
    }
}

// =====================================================
// MANUAL TEMPERATURE SCREEN
// =====================================================

void Application::drawManualTemperature()
{
    char line[17];

    display.clear();

    display.print(
        0,
        0,
        "Set Temperature"
    );

    snprintf(
        line,
        sizeof(line),
        "%3u C  < >",
        settings.temperature
    );

    display.print(
        0,
        1,
        line
    );
}

// =====================================================
// MANUAL WEIGHT
// =====================================================

void Application::handleManualWeight(
    EncoderEvent event
)
{
    if (event == ENCODER_RIGHT)
    {
        if (settings.targetWeight < MAX_WEIGHT)
        {
            settings.targetWeight += 100;
        }

        drawManualWeight();
    }

    else if (event == ENCODER_LEFT)
    {
        if (settings.targetWeight > MIN_WEIGHT)
        {
            settings.targetWeight -= 100;
        }

        drawManualWeight();
    }

    else if (event == ENCODER_CLICK)
    {
        state = STATE_READY;

        drawReadyScreen();
    }

    else if (event == ENCODER_LONG_CLICK)
    {
        state = STATE_MANUAL_TEMP;

        drawManualTemperature();
    }
}

// =====================================================
// MANUAL WEIGHT SCREEN
// =====================================================

void Application::drawManualWeight()
{
    char line[17];

    display.clear();

    display.print(
        0,
        0,
        "Target Weight"
    );

    snprintf(
        line,
        sizeof(line),
        "%4u g  < >",
        settings.targetWeight
    );

    display.print(
        0,
        1,
        line
    );
}

// =====================================================
// READY
// =====================================================

void Application::handleReady(
    EncoderEvent event
)
{
    if (event == ENCODER_CLICK)
    {
        Serial.println();
        Serial.println(
            "=============================="
        );

        Serial.println(
            " STARTING DRYING"
        );

        Serial.println(
            "=============================="
        );

        Serial.print("Temperature: ");
        Serial.print(settings.temperature);
        Serial.println(" C");

        if (settings.autoMode)
        {
            const Recipe& recipe =
                RecipeDatabase::getRecipe(
                    settings.recipeID
                );

            Serial.print("Recipe: ");
            Serial.println(recipe.name);
        }
        else
        {
            Serial.println("Mode: MANUAL");
        }

        dryer.start(
            settings.temperature);

        heaterEnabled = true;

        dryingStartTime =
            millis();

        finishScreenShown = false;

        lastDisplayUpdate =
            millis();

        state = STATE_RUNNING;

        drawRunningScreen();
    }

    else if (event == ENCODER_LONG_CLICK)
    {
        menuManager.openMain();

        state = STATE_MENU;

        drawCurrentMenu();
    }
}

// =====================================================
// READY SCREEN
// =====================================================

void Application::drawReadyScreen()
{
    char line[17];

    display.clear();

    if (settings.autoMode)
    {
        const Recipe& recipe =
            RecipeDatabase::getRecipe(
                settings.recipeID
            );

        display.print(
            0,
            0,
            recipe.name
        );
    }
    else
    {
        display.print(
            0,
            0,
            "Manual Ready"
        );
    }

    snprintf(
        line,
        sizeof(line),
        "%uC  Click=Start",
        settings.temperature
    );

    display.print(
        0,
        1,
        line
    );
}

// =====================================================
// RUNNING
// =====================================================

void Application::handleRunning(
    EncoderEvent event
)
{
    // -------------------------------------------------
    // Long click = stop
    // -------------------------------------------------

    if (event == ENCODER_LONG_CLICK)
    {
        dryer.stop();

        heaterEnabled = false;

        state = STATE_FINISHED;

        finishScreenShown = false;

        drawFinishedScreen();

        return;
    }

    // -------------------------------------------------
    // Click = pause
    // -------------------------------------------------

    if (event == ENCODER_CLICK)
    {
        dryer.stop();

        heaterEnabled = false;

        state = STATE_PAUSED;

        drawPausedScreen();

        return;
    }

    // -------------------------------------------------
    // Temperature control
    // -------------------------------------------------

    updateTemperatureControl();

    if (state != STATE_RUNNING)
        return;

    // -------------------------------------------------
    // LCD update
    // -------------------------------------------------

    if (millis() - lastDisplayUpdate >= 500)
    {
        drawRunningScreen();

        lastDisplayUpdate =
            millis();
    }
}

// =====================================================
// RUNNING SCREEN
// =====================================================

void Application::drawRunningScreen()
{
    static float lastAverage = NAN;
    static float lastHot = NAN;
    static uint8_t lastPower = 255;
    static bool firstDraw = true;

    float average =
        temperatureManager.getAverageTemperature();

    float hot =
        temperatureManager.getHotTemperature();

    uint8_t power =
        dryer.getHeaterPower();

    bool averageChanged =
        (isnan(average) != isnan(lastAverage)) ||
        (!isnan(average) &&
         !isnan(lastAverage) &&
         fabs(average - lastAverage) >= 0.1f);

    bool hotChanged =
        (isnan(hot) != isnan(lastHot)) ||
        (!isnan(hot) &&
         !isnan(lastHot) &&
         fabs(hot - lastHot) >= 0.1f);

    bool powerChanged =
        power != lastPower;

    if (!firstDraw &&
        !averageChanged &&
        !hotChanged &&
        !powerChanged)
    {
        return;
    }

    firstDraw = false;

    lastAverage = average;
    lastHot = hot;
    lastPower = power;

    char line1[17];
    char line2[17];

    if (isnan(average))
    {
        snprintf(
            line1,
            sizeof(line1),
            "AVG: --.- C");
    }
    else
    {
        snprintf(
            line1,
            sizeof(line1),
            "AVG:%5.1f C",
            average);
    }

    if (isnan(hot))
    {
        snprintf(
            line2,
            sizeof(line2),
            "HOT: --.- C");
    }
    else
    {
        snprintf(
            line2,
            sizeof(line2),
            "HOT:%5.1f C",
            hot);
    }

    display.print(
        0,
        0,
        line1
    );

    display.print(
        0,
        1,
        line2
    );
}

// =====================================================
// PAUSED
// =====================================================

void Application::handlePaused(
    EncoderEvent event
)
{
    if (event == ENCODER_CLICK)
    {
        Serial.println(
            "Resuming dryer..."
        );

        dryer.start(
            settings.temperature);

        heaterEnabled = true;

        state = STATE_RUNNING;

        drawRunningScreen();
    }

    else if (event == ENCODER_LONG_CLICK)
    {
        Serial.println(
            "Drying cancelled."
        );

        dryer.stop();

        menuManager.openMain();

        state = STATE_MENU;

        drawCurrentMenu();
    }
}

// =====================================================
// PAUSED SCREEN
// =====================================================

void Application::drawPausedScreen()
{
    display.clear();

    display.print(
        0,
        0,
        "DRYER PAUSED"
    );

    display.print(
        0,
        1,
        "Click=Resume"
    );
}

// =====================================================
// FINISHED
// =====================================================

void Application::handleFinished(
    EncoderEvent event
)
{
    if (!finishScreenShown)
    {
        drawFinishedScreen();

        finishScreenShown = true;
    }

    if (event == ENCODER_CLICK ||
        event == ENCODER_LONG_CLICK)
    {
        menuManager.openMain();

        state = STATE_MENU;

        finishScreenShown = false;

        drawCurrentMenu();
    }
}

// =====================================================
// FINISHED SCREEN
// =====================================================

void Application::drawFinishedScreen()
{
    display.clear();

    display.print(
        0,
        0,
        "Drying Finished"
    );

    display.print(
        0,
        1,
        "Click = Menu"
    );
}

// =====================================================
// ERROR
// =====================================================

void Application::handleError(
    EncoderEvent event
)
{
    if (dryer.isRunning())
    {
        dryer.stop();
    }

    if (event == ENCODER_CLICK ||
        event == ENCODER_LONG_CLICK)
    {
        menuManager.openMain();

        state = STATE_MENU;

        drawCurrentMenu();
    }
}

// =====================================================
// ERROR SCREEN
// =====================================================

void Application::drawErrorScreen()
{
    display.clear();

    display.print(
        0,
        0,
        "TEMP ERROR!"
    );

    display.print(
        0,
        1,
        "Click = Menu"
    );
}
// =====================================================
// TEMPERATURE CONTROL
// =====================================================

void Application::updateTemperatureControl()
{
    if (!dryer.isRunning())
        return;

    float chamberTemperature =
        temperatureManager.getAverageTemperature();

    float safetyTemperature =
        temperatureManager.getHotTemperature();

    // -------------------------------------------------
    // SAFETY CHECK
    // -------------------------------------------------

    if (!isnan(safetyTemperature) &&
        safetyTemperature >= MAX_SAFE_TEMP)
    {
        Serial.println();
        Serial.println("==============================");
        Serial.println(" !!! SAFETY SHUTDOWN !!!");
        Serial.println("==============================");

        Serial.print("MAX6675 Temperature: ");
        Serial.print(safetyTemperature);
        Serial.println(" C");

        dryer.stop();

        heaterEnabled = false;

        state = STATE_ERROR;

        drawErrorScreen();

        return;
    }

    // -------------------------------------------------
    // No valid chamber temperature
    // -------------------------------------------------

    if (isnan(chamberTemperature))
    {
        return;
    }

    // -------------------------------------------------
    // HEATER CONTROL
    // -------------------------------------------------

    float targetTemperature =
        settings.temperature;

    // Heater OFF when target is reached
    if (heaterEnabled &&
        chamberTemperature >= targetTemperature)
    {
        dryer.heaterOff();

        heaterEnabled = false;

        Serial.print("[CONTROL] Heater OFF | Chamber: ");
        Serial.print(chamberTemperature);
        Serial.println(" C");
    }

    // Heater ON when temperature falls below
    // target - hysteresis
    else if (!heaterEnabled &&
             chamberTemperature <=
             targetTemperature - TEMP_HYSTERESIS)
    {
        dryer.heaterOn();

        heaterEnabled = true;

        Serial.print("[CONTROL] Heater ON | Chamber: ");
        Serial.print(chamberTemperature);
        Serial.println(" C");
    }
}