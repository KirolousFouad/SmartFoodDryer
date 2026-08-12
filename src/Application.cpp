#include "Application.h"

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

      dryer(
          CIRCULATION_FAN_PIN,
          COOLING_FAN_PWM_PIN),

      scr(
          SCR_INCREASE_PIN,
          SCR_DECREASE_PIN),

      temperatureManager(
          DS18B20_PIN,
          MAX6675_SCK,
          MAX6675_CS,
          MAX6675_SO),

      settings(),

      state(STATE_MENU),
      weightSensor(
          HX711_DOUT,
          HX711_SCK),

    startingWeight(0.0f),
    currentWeight(0.0f),
    targetWeight(0.0f),
    weightTargetStartTime(0),
    weightTargetReached(false),

      lastHeartbeat(0),
      dryingStartTime(0),
      lastDryerUpdate(0),
      lastDisplayUpdate(0),
      lastTemperatureControl(0),
      targetReachedStartTime(0),
      lastSCRPulse(0),

      heaterEnabled(false),
      finishScreenShown(false),
      targetReached(false),

      currentSoftwarePower(0),
      targetSoftwarePower(0),

      scrResetInProgress(false),
      scrResetStepsRemaining(0)
{
    settings.temperature = 60;
    settings.targetWeight = 1000;
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

    // =================================================
    // DISPLAY
    // =================================================

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

    // =================================================
    // ENCODER
    // =================================================

    Serial.println("[INIT] Encoder...");

    encoder.begin();

    Serial.println("[INIT] Encoder OK");

    // =================================================
    // MENU
    // =================================================

    Serial.println("[INIT] Menu Manager...");

    menuManager.begin();

    Serial.println("[INIT] Menu Manager OK");

    // =================================================
    // DRYER
    // =================================================

    Serial.println("[INIT] Dryer...");

    dryer.begin();

    Serial.println("[INIT] Dryer OK");

    // =================================================
    // TEMPERATURE
    // =================================================

    Serial.println("[INIT] Temperature Manager...");

    temperatureManager.begin();

    Serial.println("[INIT] Temperature Manager OK");

    // =================================================
    // SCR
    // =================================================

    Serial.println("[INIT] SCR...");

    scr.begin();

    Serial.println("[INIT] SCR OK");

    // =================================================
    // HX711
    // =================================================

    Serial.println("[INIT] Weight Sensor...");

    weightSensor.begin();

    Serial.println("[INIT] Weight Sensor OK");
    // =================================================
    // INITIAL SCR RESET
    // =================================================

    Serial.println();
    Serial.println("[INIT] Resetting SCR to 0%...");

    /*
     * The physical SCR behaves like a push button.
     *
     * We don't know its physical value after power-up.
     * Therefore we send 100 decrease pulses.
     *
     * This is done directly at startup.
     */
    scr.resetToZero();

    currentSoftwarePower = 0;
    targetSoftwarePower = 0;

    scrResetInProgress = false;
    scrResetStepsRemaining = 0;

    Serial.println("[INIT] SCR reset complete.");

    // =================================================
    // INITIAL STATE
    // =================================================

    state = STATE_MENU;

    lastHeartbeat = millis();

    dryingStartTime = 0;

    lastDryerUpdate = millis();

    lastDisplayUpdate = millis();

    lastTemperatureControl = millis();

    targetReachedStartTime = 0;

    lastSCRPulse = millis();

    heaterEnabled = false;

    finishScreenShown = false;

    targetReached = false;

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
    // =================================================
    // ENCODER
    // =================================================

    encoder.update();

    EncoderEvent event =
        encoder.getEvent();
    // =================================================
    // WEIGHT SENSOR
    // =================================================

    weightSensor.update();

    // =================================================
    // TEMPERATURE
    // =================================================

    static unsigned long lastTemperatureUpdate = 0;

    if (millis() - lastTemperatureUpdate >= 1000)
    {
        lastTemperatureUpdate = millis();

        temperatureManager.update();

        // TEMP DEBUG
        Serial.print("[TEMP DEBUG] AVG = ");
        Serial.print(temperatureManager.getAverageTemperature());

        Serial.print(" C | HOT = ");
        Serial.print(temperatureManager.getHotTemperature());

        Serial.println(" C");

        // Keep temperature control
        if (state == STATE_RUNNING)
        {
            updateTemperatureControl();
        }
    }

    // =================================================
    // DRYER
    // =================================================

    if (millis() - lastDryerUpdate >= 50)
    {
        dryer.update();

        lastDryerUpdate = millis();
    }

    // =================================================
    // SCR
    //
    // IMPORTANT:
    // SCR control is NOT limited to STATE_RUNNING.
    //
    // This allows the SCR to return to zero after:
    // - Finish
    // - Error
    // - Pause cancellation
    // =================================================

    updateSCRControl();

    // =================================================
    // APPLICATION STATE
    // =================================================

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

            updateWeightControl();

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

    // =================================================
    // HEARTBEAT
    // =================================================

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

void Application::updateTemperatureControl()
{
    if (!dryer.isRunning())
    {
        return;
    }

    float chamberTemperature =
        temperatureManager.getAverageTemperature();

    float safetyTemperature =
        temperatureManager.getHotTemperature();

    // =================================================
    // SAFETY
    // =================================================

    if (!isnan(safetyTemperature) &&
        safetyTemperature >= MAX_SAFE_TEMP)
    {
        Serial.println();
        Serial.println("==============================");
        Serial.println(" !!! SAFETY SHUTDOWN !!!");
        Serial.println("==============================");

        Serial.print("Temperature: ");
        Serial.print(safetyTemperature);
        Serial.println(" C");

        dryer.stop();

        heaterEnabled = false;

        targetReached = false;

        targetReachedStartTime = 0;

        targetSoftwarePower = 0;

        startSCRReset();

        state = STATE_ERROR;

        drawErrorScreen();

        return;
    }

    // =================================================
    // INVALID TEMPERATURE
    // =================================================

    if (isnan(chamberTemperature))
    {
        Serial.println(
            "[CONTROL] Invalid chamber temperature"
        );

        return;
    }

    float targetTemperature =
        settings.temperature;

    // =================================================
    // TARGET REACHED
    // =================================================

    if (chamberTemperature >= targetTemperature)
    {
        if (!targetReached)
        {
            targetReached = true;

            targetReachedStartTime =
                millis();

            Serial.println();
            Serial.println(
                "[CONTROL] Target temperature reached"
            );

            Serial.print(
                "[CONTROL] Chamber: "
            );

            Serial.print(
                chamberTemperature
            );

            Serial.println(" C");

            Serial.println(
                "[CONTROL] Starting 20 second hold"
            );
        }

        /*
         * Heater power becomes zero.
         *
         * The SCR will move down one physical pulse
         * at a time.
         */
        targetSoftwarePower = 0;

        return;
    }

    // =================================================
    // BELOW TARGET
    // =================================================

    if (targetReached)
    {
        targetReached = false;

        targetReachedStartTime = 0;

        Serial.println(
            "[CONTROL] Temperature dropped below target"
        );
    }

    float difference =
        targetTemperature -
        chamberTemperature;

    uint8_t desiredPower =
        calculateHeaterPower(
            chamberTemperature,
            targetTemperature
        );

    targetSoftwarePower =
        desiredPower;

    Serial.print("[CONTROL] AVG: ");
    Serial.print(chamberTemperature);

    Serial.print(" C | Target: ");
    Serial.print(targetTemperature);

    Serial.print(" C | Difference: ");
    Serial.print(difference);

    Serial.print(" C | SCR Target: ");
    Serial.print(targetSoftwarePower);

    Serial.println("%");
}

// =====================================================
// HEATER POWER CALCULATION
// =====================================================

uint8_t Application::calculateHeaterPower(
    float currentTemperature,
    float targetTemperature
)
{
    float difference =
        targetTemperature -
        currentTemperature;

    if (difference <= 0.0f)
    {
        return 0;
    }

    if (difference >= 10.0f)
    {
        return 100;
    }

    if (difference >= 5.0f)
    {
        return 70;
    }

    if (difference >= 2.0f)
    {
        return 40;
    }

    return 20;
}

// =====================================================
// SCR CONTROL
// =====================================================

void Application::updateSCRControl()
{
    // =================================================
    // RESET HAS PRIORITY
    // =================================================

    if (scrResetInProgress)
    {
        updateSCRReset();

        return;
    }

    // =================================================
    // ALREADY AT TARGET
    // =================================================

    if (currentSoftwarePower ==
        targetSoftwarePower)
    {
        return;
    }

    // =================================================
    // ONE PHYSICAL PULSE AT A TIME
    // =================================================

    /*
     * Never send another pulse until the previous
     * physical button action has completed.
     */

    if (millis() - lastSCRPulse < 300)
    {
        return;
    }

    lastSCRPulse = millis();

    // =================================================
    // INCREASE
    // =================================================

    if (currentSoftwarePower <
        targetSoftwarePower)
    {
        increaseSCRPulse();
    }

    // =================================================
    // DECREASE
    // =================================================

    else
    {
        decreaseSCRPulse();
    }
}

// =====================================================
// INCREASE SCR ONE PHYSICAL STEP
// =====================================================

void Application::increaseSCRPulse()
{
    if (currentSoftwarePower >= 100)
    {
        currentSoftwarePower = 100;

        return;
    }

    /*
     * SCRController::increase()
     *
     * = ONE physical button press
     *
     * = ONE SCR %
     */

    scr.increase();

    currentSoftwarePower++;

    Serial.print(
        "[SCR] Physical pulse +1 -> "
    );

    Serial.print(
        currentSoftwarePower
    );

    Serial.println("%");
}

// =====================================================
// DECREASE SCR ONE PHYSICAL STEP
// =====================================================

void Application::decreaseSCRPulse()
{
    if (currentSoftwarePower == 0)
    {
        return;
    }

    /*
     * SCRController::decrease()
     *
     * = ONE physical button press
     *
     * = ONE SCR %
     */

    scr.decrease();

    currentSoftwarePower--;

    Serial.print(
        "[SCR] Physical pulse -1 -> "
    );

    Serial.print(
        currentSoftwarePower
    );

    Serial.println("%");
}

// =====================================================
// START SCR RESET
// =====================================================

void Application::startSCRReset()
{
    if (scrResetInProgress)
    {
        return;
    }

    Serial.println(
        "[SCR] Starting non-blocking reset to 0%"
    );

    /*
     * We don't know the actual physical SCR value.
     *
     * Send enough decrease pulses to guarantee zero.
     *
     * One pulse every update cycle.
     */
    scrResetInProgress = true;

    scrResetStepsRemaining = 100;

    targetSoftwarePower = 0;

    lastSCRPulse = millis();
}

// =====================================================
// UPDATE SCR RESET
// =====================================================

void Application::updateSCRReset()
{
    if (!scrResetInProgress)
    {
        return;
    }

    /*
     * Wait between physical button presses.
     *
     * This keeps:
     * - encoder responsive
     * - LCD responsive
     * - temperature readings running
     */

    if (millis() - lastSCRPulse < 300)
    {
        return;
    }

    lastSCRPulse = millis();

    if (scrResetStepsRemaining > 0)
    {
        scr.decrease();

        scrResetStepsRemaining--;

        currentSoftwarePower = 0;

        Serial.print(
            "[SCR RESET] Pulse sent | Remaining: "
        );

        Serial.println(
            scrResetStepsRemaining
        );
    }

    if (scrResetStepsRemaining == 0)
    {
        scrResetInProgress = false;

        currentSoftwarePower = 0;

        targetSoftwarePower = 0;

        Serial.println(
            "[SCR RESET] Physical SCR = 0%"
        );
    }
}

// =====================================================
// STOP SCR MOVEMENT
// =====================================================

void Application::stopSCRMovement()
{
    targetSoftwarePower =
        currentSoftwarePower;
}

// =====================================================
// TARGET TEMPERATURE HOLD
// =====================================================

void Application::checkTargetTemperatureHold()
{
    if (!targetReached)
    {
        return;
    }

    /*
     * Make sure the heater is already OFF.
     */

    targetSoftwarePower = 0;

    if (millis() -
        targetReachedStartTime >= 20000UL)
    {
        Serial.println();
        Serial.println("==============================");
        Serial.println(" SIMULATION FINISHED");
        Serial.println("==============================");

        Serial.println(
            "Target temperature maintained for 20 seconds."
        );

        dryer.stop();

        heaterEnabled = false;

        targetSoftwarePower = 0;

        /*
         * Start SCR reset but DO NOT block.
         */
        startSCRReset();

        state = STATE_FINISHED;

        finishScreenShown = false;

        drawFinishedScreen();
    }
}

// =====================================================
// MENU
// =====================================================

void Application::handleMenu(
    EncoderEvent event
)
{
    Menu* menu =
        menuManager.currentMenu();

    if (menu == nullptr)
    {
        return;
    }

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
            Serial.println("==============================");
            Serial.println(" RECIPE SELECTED");
            Serial.println("==============================");

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
    {
        return;
    }

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
        if (settings.temperature < 120)
        {
            settings.temperature++;
        }

        drawManualTemperature();
    }

    else if (event == ENCODER_LEFT)
    {
        if (settings.temperature > 0)
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
        if (settings.targetWeight < 10000)
        {
            settings.targetWeight += 100;
        }

        drawManualWeight();
    }

    else if (event == ENCODER_LEFT)
    {
        if (settings.targetWeight > 100)
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
        /*
         * DO NOT start the dryer while SCR reset
         * is still active.
         */

        if (scrResetInProgress)
        {
            Serial.println(
                "[READY] Waiting for SCR reset..."
            );

            return;
        }

        Serial.println();
        Serial.println("==============================");
        Serial.println(" STARTING DRYING");
        Serial.println("==============================");

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

        // =============================================
        // START DRYER
        // =============================================

        dryer.start(
            settings.temperature
        );

        heaterEnabled = true;

        // =============================================
        // RESET SOFTWARE CONTROL
        // =============================================

        currentSoftwarePower = 0;

        targetSoftwarePower = 0;

        targetReached = false;

        targetReachedStartTime = 0;

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
        "%uC Click=Start",
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

void Application::handleRunning(EncoderEvent event)
{
    // -------------------------------------------------
    // ENCODER CLICK = PAUSE
    // -------------------------------------------------

    if (event == ENCODER_CLICK)
    {
        Serial.println("[RUNNING] Pause requested");

        dryer.stop();
        heaterEnabled = false;

        // Stop increasing/decreasing SCR target.
        // Current physical SCR value is left unchanged
        // while paused.
        stopSCRMovement();

        state = STATE_PAUSED;

        drawPausedScreen();

        return;
    }

    // -------------------------------------------------
    // LONG CLICK = FINISH
    // -------------------------------------------------

    if (event == ENCODER_LONG_CLICK)
    {
        Serial.println("[RUNNING] Finish requested");

        dryer.stop();
        heaterEnabled = false;

        // Immediately request SCR = 0%.
        targetSoftwarePower = 0;

        state = STATE_FINISHED;
        finishScreenShown = false;

        drawFinishedScreen();

        return;
    }

    // -------------------------------------------------
    // TEMPERATURE HOLD
    // -------------------------------------------------

    checkTargetTemperatureHold();

    if (state != STATE_RUNNING)
    {
        return;
    }

    // -------------------------------------------------
    // LCD UPDATE
    // -------------------------------------------------

    if (millis() - lastDisplayUpdate >= 500)
    {
        drawRunningScreen();

        lastDisplayUpdate = millis();
    }
}
void Application::drawRunningScreen()
{
    float average =
        temperatureManager.getAverageTemperature();

    float hot =
        temperatureManager.getHotTemperature();

    char line1[17];
    char line2[17];

    // =================================================
    // LINE 1
    // Average temperature + target temperature
    // =================================================

    if (isnan(average))
    {
        snprintf(
            line1,
            sizeof(line1),
            "AVG: --.-C T:%3uC",
            settings.temperature
        );
    }
    else
    {
        char avgText[8];

        dtostrf(
            average,
            4,
            1,
            avgText
        );

        snprintf(
            line1,
            sizeof(line1),
            "AVG:%sC T:%3uC",
            avgText,
            settings.temperature
        );
    }

    // =================================================
    // LINE 2
    // Hot temperature
    // =================================================

    if (isnan(hot))
    {
        snprintf(
            line2,
            sizeof(line2),
            "HOT: --.-C"
        );
    }
    else
    {
        char hotText[8];

        dtostrf(
            hot,
            4,
            1,
            hotText
        );

        snprintf(
            line2,
            sizeof(line2),
            "HOT:%sC",
            hotText
        );
    }

    // =================================================
    // LCD
    // Clear both rows completely
    // =================================================

    display.print(
        0,
        0,
        "                "
    );

    display.print(
        0,
        1,
        "                "
    );

    // =================================================
    // Write new values
    // =================================================

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
            settings.temperature
        );

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

        heaterEnabled = false;

        targetSoftwarePower = 0;

        /*
         * Reset SCR physically.
         */

        startSCRReset();

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
    /*
     * ALWAYS keep requesting zero.
     */

    targetSoftwarePower = 0;

    /*
     * SCR reset continues in the background
     * through updateSCRControl().
     */

    if (!finishScreenShown)
    {
        drawFinishedScreen();

        finishScreenShown = true;
    }

    /*
     * Encoder remains immediately responsive.
     */

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

    heaterEnabled = false;

    targetSoftwarePower = 0;

    /*
     * Keep resetting SCR in background.
     */

    if (!scrResetInProgress &&
        currentSoftwarePower > 0)
    {
        startSCRReset();
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
// =========================================================
// WEIGHT CONTROL
// =========================================================

void Application::updateWeightControl()
{
    // -------------------------------------------------
    // Safety check
    // -------------------------------------------------

    if (!weightSensor.isReady())
    {
        return;
    }

    if (weightSensor.isTaring())
    {
        return;
    }

    // -------------------------------------------------
    // Read current filtered weight
    // -------------------------------------------------

    currentWeight =
        weightSensor.getWeight();

    // -------------------------------------------------
    // Invalid reading
    // -------------------------------------------------

    if (isnan(currentWeight))
    {
        return;
    }

    // -------------------------------------------------
    // Check target weight
    // -------------------------------------------------

    checkTargetWeight();
}
// =========================================================
// CAPTURE STARTING WEIGHT
// =========================================================

void Application::captureStartingWeight()
{
    if (!weightSensor.isReady())
    {
        Serial.println(
            "[WEIGHT] Cannot capture starting weight"
        );

        return;
    }

    startingWeight =
        weightSensor.getWeight();

    currentWeight =
        startingWeight;

    weightTargetStartTime = 0;

    weightTargetReached = false;

    Serial.print(
        "[WEIGHT] Starting weight: "
    );

    Serial.print(
        startingWeight,
        2
    );

    Serial.println(
        " g"
    );
}
// =========================================================
// CHECK TARGET WEIGHT
// =========================================================

void Application::checkTargetWeight()
{
    // -------------------------------------------------
    // Target must be valid
    // -------------------------------------------------

    if (targetWeight <= 0.0f)
    {
        return;
    }

    // -------------------------------------------------
    // Already finished
    // -------------------------------------------------

    if (weightTargetReached)
    {
        return;
    }

    // -------------------------------------------------
    // Check whether target has been reached
    // -------------------------------------------------

    if (currentWeight <= targetWeight)
    {
        // Start confirmation timer
        if (weightTargetStartTime == 0)
        {
            weightTargetStartTime =
                millis();

            Serial.println(
                "[WEIGHT] Target reached - "
                "starting confirmation"
            );
        }

        // -------------------------------------------------
        // Require the target condition to remain valid
        // -------------------------------------------------

        constexpr unsigned long TARGET_HOLD_TIME =
            5000UL;

        if (millis() -
            weightTargetStartTime >=
            TARGET_HOLD_TIME)
        {
            weightTargetReached = true;

            Serial.println(
                "[WEIGHT] Target weight confirmed"
            );

            finishDrying();
        }
    }
    else
    {
        // Weight went above target again.
        // Cancel confirmation timer.

        if (weightTargetStartTime != 0)
        {
            Serial.println(
                "[WEIGHT] Target confirmation cancelled"
            );
        }

        weightTargetStartTime = 0;
    }
}
// =========================================================
// FINISH DRYING
// =========================================================

void Application::finishDrying()
{
    Serial.println();
    Serial.println(
        "================================"
    );

    Serial.println(
        "[DRYING] TARGET WEIGHT REACHED"
    );

    Serial.println(
        "================================"
    );

    // -------------------------------------------------
    // Disable heater
    // -------------------------------------------------

    heaterEnabled = false;

    targetSoftwarePower = 0;

    // -------------------------------------------------
    // Force SCR shutdown
    // -------------------------------------------------

    startSCRReset();

    // -------------------------------------------------
    // Save final weight
    // -------------------------------------------------

    currentWeight =
        weightSensor.getWeight();

    Serial.print(
        "[DRYING] Starting weight: "
    );

    Serial.print(
        startingWeight,
        2
    );

    Serial.println(
        " g"
    );

    Serial.print(
        "[DRYING] Target weight: "
    );

    Serial.print(
        targetWeight,
        2
    );

    Serial.println(
        " g"
    );

    Serial.print(
        "[DRYING] Final weight: "
    );

    Serial.print(
        currentWeight,
        2
    );

    Serial.println(
        " g"
    );

    // -------------------------------------------------
    // Change application state
    // -------------------------------------------------

    state = STATE_FINISHED;

    finishScreenShown = false;
}