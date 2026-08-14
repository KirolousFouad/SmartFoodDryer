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

      buzzer(
          BUZZER_PIN),

      settings(),

      state(STATE_MENU),
      weightSensor(
          HX711_DOUT,
          HX711_SCK),

      coolingAfterDrying(false),
      coolingComplete(false),

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
      targetSoftwarePower(0),

      scrResetInProgress(false),
      scrResetStepsRemaining(0),
      circulationFanOn(false),
      coolingFanPower(0),
      lastCoolingFanUpdate(0)
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
    // BUZZER
    // =================================================

    Serial.println("[INIT] Buzzer...");

    buzzer.begin();

    Serial.println("[INIT] Buzzer OK");
    // =========================================================
    // FANS
    // =========================================================

    Serial.println("[INIT] Fans...");

    pinMode(
        CIRCULATION_FAN_PIN,
        OUTPUT);

    pinMode(
        COOLING_FAN_PWM_PIN,
        OUTPUT);

    // Start with both fans OFF

    digitalWrite(
        CIRCULATION_FAN_PIN,
        LOW);

    analogWrite(
        COOLING_FAN_PWM_PIN,
        0);

    circulationFanOn = false;

    coolingFanPower = 0;

    lastCoolingFanUpdate = 0;

    Serial.println("[INIT] Fans OK");
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

    coolingAfterDrying = false;
    coolingComplete = false;

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

    // =================================================
    // BUZZER
    // =================================================

    buzzer.update();

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
    // FANS
    // =================================================

    updateFanControl();

    updatePostDryingCooling();

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
        if (state != STATE_ERROR)
        {
            buzzer.error();
        }
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
        buzzer.warning();
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

    if (scr.getPower() ==
        targetSoftwarePower)
    {
        return;
    }

    // =================================================
    // ONE PHYSICAL PULSE AT A TIME
    // =================================================

    if (millis() - lastSCRPulse < 300)
    {
        return;
    }

    lastSCRPulse = millis();

    // =================================================
    // MOVE ONE STEP
    // =================================================

    scr.moveOneStepToward(
        targetSoftwarePower
    );
}

// =====================================================
// START SCR RESET
// =====================================================
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
     * We do not know the actual physical SCR position.
     *
     * Therefore send 100 physical decrease pulses.
     *
     * The SCRController::resetStep() method always sends
     * a physical decrease pulse, even when its software
     * power is already 0%.
     */

    scrResetInProgress = true;

    scrResetStepsRemaining = 100;

    targetSoftwarePower = 0;

    lastSCRPulse = millis();
}
// =====================================================
// UPDATE SCR RESET
// =====================================================
// =====================================================
// UPDATE SCR RESET
// =====================================================

void Application::updateSCRReset()
{
    if (!scrResetInProgress)
    {
        return;
    }

    // -------------------------------------------------
    // Wait between physical reset pulses
    // -------------------------------------------------

    if (millis() - lastSCRPulse < 300)
    {
        return;
    }

    lastSCRPulse = millis();

    // -------------------------------------------------
    // Send one physical decrease pulse
    // -------------------------------------------------

    if (scrResetStepsRemaining > 0)
    {
        scr.resetStep();

        scrResetStepsRemaining--;

        Serial.print(
            "[SCR RESET] Pulse sent | Remaining: "
        );

        Serial.println(
            scrResetStepsRemaining
        );
    }

    // -------------------------------------------------
    // Reset complete
    // -------------------------------------------------

    if (scrResetStepsRemaining == 0)
    {
        scrResetInProgress = false;

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
        scr.getPower();
}
// =====================================================
// TARGET TEMPERATURE HOLD
// =====================================================

void Application::checkTargetTemperatureHold()
{
    /*
     * Temperature is used for heater control only.
     *
     * Drying completion is determined by
     * the target weight.
     *
     * Therefore this function must NOT transition
     * the application to STATE_FINISHED.
     */

    if (!targetReached)
    {
        return;
    }

    /*
     * Keep heater power at zero while the chamber
     * temperature is at or above the target.
     */

    targetSoftwarePower = 0;
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
        buzzer.beepShort();
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
        buzzer.beepShort();
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
        buzzer.beepShort();
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
        // =================================================
        // SAFETY CHECKS
        // =================================================

        if (scrResetInProgress)
        {
            Serial.println(
                "[READY] Waiting for SCR reset..."
            );

            return;
        }

        if (!weightSensor.isReady())
        {
            Serial.println(
                "[READY] ERROR: Weight sensor not ready"
            );

            state = STATE_ERROR;
            drawErrorScreen();

            return;
        }

        if (weightSensor.isTaring())
        {
            Serial.println(
                "[READY] Waiting for weight sensor tare..."
            );

            return;
        }

        // =================================================
        // CAPTURE TARGET WEIGHT
        // =================================================

        targetWeight =
            settings.targetWeight;

        if (targetWeight <= 0.0f)
        {
            Serial.println(
                "[READY] ERROR: Invalid target weight"
            );

            state = STATE_ERROR;
            drawErrorScreen();

            return;
        }

        // =================================================
        // CAPTURE STARTING WEIGHT
        // =================================================

        captureStartingWeight();

        // -------------------------------------------------
        // Verify starting weight
        // -------------------------------------------------

        if (startingWeight <= 0.0f)
        {
            Serial.println(
                "[READY] ERROR: Invalid starting weight"
            );

            state = STATE_ERROR;
            drawErrorScreen();

            return;
        }

        // =================================================
        // TARGET MUST BE LOWER THAN STARTING WEIGHT
        // =================================================

        if (targetWeight >= startingWeight)
        {
            Serial.println();
            Serial.println(
                "[READY] ERROR: Target weight must be"
            );
            Serial.println(
                "[READY] lower than starting weight"
            );

            Serial.print(
                "[READY] Starting: "
            );

            Serial.print(
                startingWeight,
                2
            );

            Serial.println(" g");

            Serial.print(
                "[READY] Target: "
            );

            Serial.print(
                targetWeight,
                2
            );

            Serial.println(" g");

            state = STATE_ERROR;
            drawErrorScreen();

            return;
        }

        // =================================================
        // STARTING INFORMATION
        // =================================================

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

        Serial.print(
            "Temperature: "
        );

        Serial.print(
            settings.temperature
        );

        Serial.println(" C");

        Serial.print(
            "Starting weight: "
        );

        Serial.print(
            startingWeight,
            2
        );

        Serial.println(" g");

        Serial.print(
            "Target weight: "
        );

        Serial.print(
            targetWeight,
            2
        );

        Serial.println(" g");

        // =================================================
        // RECIPE / MODE
        // =================================================

        if (settings.autoMode)
        {
            const Recipe& recipe =
                RecipeDatabase::getRecipe(
                    settings.recipeID
                );

            Serial.print(
                "Recipe: "
            );

            Serial.println(
                recipe.name
            );
        }
        else
        {
            Serial.println(
                "Mode: MANUAL"
            );
        }

        // =================================================
        // START DRYER
        // =================================================

        dryer.start(
            settings.temperature
        );

        heaterEnabled = true;
        buzzer.success();

        // =================================================
        // RESET SOFTWARE CONTROL
        // =================================================


        targetSoftwarePower = 0;

        targetReached = false;

        targetReachedStartTime = 0;

        weightTargetStartTime = 0;

        weightTargetReached = false;

        dryingStartTime =
            millis();

        finishScreenShown = false;

        lastDisplayUpdate =
            millis();

        // =================================================
        // ENTER RUNNING STATE
        // =================================================

        state =
            STATE_RUNNING;

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
        buzzer.beepShort();
        Serial.println("[RUNNING] Pause requested");

        dryer.stop();

        heaterEnabled = false;

        // Force heater power request to zero.
        targetSoftwarePower = 0;

        // Physically ramp SCR down to zero.
        startSCRReset();

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

        // Force heater power request to zero.
        targetSoftwarePower = 0;

        // Physically reset SCR toward zero.
        startSCRReset();

        // -------------------------------------------------
        // Start post-drying cooling
        // -------------------------------------------------

        coolingAfterDrying = true;
        coolingComplete = false;

        setCirculationFan(false);

        setCoolingFanPower(100);

        Serial.println(
            "[DRYING] Starting post-drying cooling");

        Serial.print(
            "[DRYING] Cooling until chamber reaches ");

        Serial.print(
            COOLING_FINISH_TEMP);

        Serial.println(" C");

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

    char line1[17];
    char line2[17];

    // =================================================
    // LINE 1
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
    // LINE 2 - WEIGHT
    // =================================================

    float currentWeight =
        weightSensor.getWeight();

    if (currentWeight < 0.0f)
    {
        currentWeight = 0.0f;
    }

    char weightText[8];

    dtostrf(
        currentWeight,
        4,
        0,
        weightText
    );

    snprintf(
        line2,
        sizeof(line2),
        "W:%sg >%4ug",
        weightText,
        (unsigned int)targetWeight
    );

    // =================================================
    // LCD
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
    // =================================================
    // RESUME
    // =================================================

    if (event == ENCODER_CLICK)
    {
        
        Serial.println(
            "[PAUSED] Resume requested"
        );

        // Wait until the SCR has physically returned to zero.
        if (scrResetInProgress)
        {
            Serial.println(
                "[PAUSED] Waiting for SCR reset..."
            );

            return;
        }
        buzzer.beepShort();

        // Restart dryer.
        dryer.start(
            settings.temperature
        );

        heaterEnabled = true;

        // Start from zero SCR power.
        targetSoftwarePower = 0;

        // Re-enable temperature control from a clean state.
        targetReached = false;
        targetReachedStartTime = 0;

        state = STATE_RUNNING;

        lastTemperatureControl = millis();

        drawRunningScreen();

        return;
    }

    // =================================================
    // CANCEL
    // =================================================

    else if (event == ENCODER_LONG_CLICK)
    {
        Serial.println(
            "Drying cancelled."
        );

        dryer.stop();

        heaterEnabled = false;

        targetSoftwarePower = 0;

        // Reset SCR physically.
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
    // Always request SCR = 0%.
    targetSoftwarePower = 0;

    if (!finishScreenShown)
    {
        drawFinishedScreen();

        finishScreenShown = true;
    }

    if (event == ENCODER_CLICK ||
        event == ENCODER_LONG_CLICK)
    {
        // Do not leave the finished state until
        // the physical SCR has reached zero.
        if (scrResetInProgress)
        {
            Serial.println(
                "[FINISHED] Waiting for SCR shutdown..."
            );

            return;
        }

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
// =====================================================
// ERROR
// =====================================================
void Application::handleError(
    EncoderEvent event
)
{
    // =================================================
    // FORCE DRYER OFF
    // =================================================

    if (dryer.isRunning())
    {
        dryer.stop();
    }

    heaterEnabled = false;

    // =================================================
    // FORCE SCR TO ZERO
    // =================================================

    targetSoftwarePower = 0;

    if (!scrResetInProgress &&
        scr.getPower() > 0)
    {
        startSCRReset();
    }

    // =================================================
    // RETURN TO MENU
    // =================================================

    if (event == ENCODER_CLICK ||
        event == ENCODER_LONG_CLICK)
    {
        if (scrResetInProgress)
        {
            Serial.println(
                "[ERROR] Waiting for SCR shutdown..."
            );

            return;
        }

        buzzer.stop();

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
// =====================================================
// TARGET WEIGHT CHECK
// =====================================================

void Application::checkTargetWeight()
{
    // =================================================
    // SAFETY
    // =================================================

    if (state != STATE_RUNNING)
    {
        return;
    }

    if (!weightSensor.isReady())
    {
        return;
    }

    if (weightSensor.isTaring())
    {
        return;
    }

    // =================================================
    // CURRENT WEIGHT
    // =================================================

    currentWeight =
        weightSensor.getWeight();

    // -------------------------------------------------
    // Protect against invalid negative readings
    // -------------------------------------------------

    if (currentWeight < 0.0f)
    {
        currentWeight = 0.0f;
    }

    // =================================================
    // VALIDATE TARGET
    // =================================================

    if (startingWeight <= 0.0f)
    {
        return;
    }

    if (targetWeight <= 0.0f)
    {
        return;
    }

    /*
     * Target weight must be lower than
     * the starting weight.
     */

    if (targetWeight >= startingWeight)
    {
        return;
    }

    // =================================================
    // TARGET NOT YET REACHED
    // =================================================

    if (currentWeight > targetWeight)
    {
        /*
         * If the weight rises above the target again,
         * cancel the target confirmation timer.
         */

        if (weightTargetStartTime != 0)
        {
            Serial.println(
                "[WEIGHT] Target no longer reached"
            );
        }

        weightTargetStartTime = 0;

        weightTargetReached = false;

        return;
    }

    // =================================================
    // TARGET WEIGHT REACHED
    // =================================================

    if (currentWeight <= targetWeight)
    {
        // -------------------------------------------------
        // First detection
        // -------------------------------------------------

        if (weightTargetStartTime == 0)
        {
            weightTargetStartTime =
                millis();

            weightTargetReached =
                false;

            Serial.println();
            Serial.println(
                "[WEIGHT] Target weight reached"
            );

            Serial.print(
                "[WEIGHT] Current: "
            );

            Serial.print(
                currentWeight,
                2
            );

            Serial.println(
                " g"
            );

            Serial.println(
                "[WEIGHT] Waiting for stable weight..."
            );
        }

        // -------------------------------------------------
        // Weight must be stable
        // -------------------------------------------------

        if (!weightSensor.isStable())
        {
            /*
             * The weight is below the target,
             * but the load cell is still moving.
             *
             * Do NOT finish the drying cycle.
             */

            weightTargetReached =
                false;

            Serial.println(
                "[WEIGHT] Target reached but weight is NOT stable"
            );

            return;
        }

        // =================================================
        // STABLE TARGET
        // =================================================

        if (millis() -
            weightTargetStartTime >= 5000UL)
        {
            weightTargetReached =
                true;

            Serial.println();
            Serial.println(
                "=============================="
            );

            Serial.println(
                "[WEIGHT] TARGET WEIGHT CONFIRMED"
            );

            Serial.println(
                "=============================="
            );

            Serial.print(
                "[WEIGHT] Starting: "
            );

            Serial.print(
                startingWeight,
                2
            );

            Serial.println(
                " g"
            );

            Serial.print(
                "[WEIGHT] Target: "
            );

            Serial.print(
                targetWeight,
                2
            );

            Serial.println(
                " g"
            );

            Serial.print(
                "[WEIGHT] Final: "
            );

            Serial.print(
                currentWeight,
                2
            );

            Serial.println(
                " g"
            );

            // -------------------------------------------------
            // Finish drying
            // -------------------------------------------------

            finishDrying();
        }
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

    // -------------------------------------------------
// Start post-drying cooling
// -------------------------------------------------

    coolingAfterDrying = true;
    coolingComplete = false;

    setCirculationFan(false);

    setCoolingFanPower(100);

    Serial.println(
        "[DRYING] Starting post-drying cooling");

    Serial.print(
        "[DRYING] Cooling until chamber reaches ");

    Serial.print(
        COOLING_FINISH_TEMP);

    Serial.println(" C");
    buzzer.success();
}
// =========================================================
// CIRCULATION FAN
// =========================================================

void Application::setCirculationFan(bool on)
{
    if (on)
    {
        digitalWrite(
            CIRCULATION_FAN_PIN,
            HIGH
        );

        circulationFanOn = true;
    }
    else
    {
        digitalWrite(
            CIRCULATION_FAN_PIN,
            LOW
        );

        circulationFanOn = false;
    }
}
// =========================================================
// COOLING FAN PWM
// =========================================================
void Application::setCoolingFanPower(uint8_t power)
{
    if (power > 100)
    {
        power = 100;
    }

    coolingFanPower = power;

    // 0% = PWM 0
    // 100% = PWM 255
    uint8_t pwmValue = map(
        power,
        0,
        100,
        0,
        255
    );

    analogWrite(
        COOLING_FAN_PWM_PIN,
        pwmValue
    );

}
// =========================================================
// FAN CONTROL
// =========================================================

void Application::updateFanControl()
{
    // =====================================================
    // CIRCULATION FAN
    // =====================================================

    if (state == STATE_RUNNING)
    {
        setCirculationFan(true);
    }
    else
    {
        setCirculationFan(false);
    }

    // =====================================================
    // COOLING FAN
    // =====================================================

    // Cooling fan is only needed during drying.
    if (coolingAfterDrying)
    {
        setCirculationFan(false);
        setCoolingFanPower(100);

        return;
    }

    if (state != STATE_RUNNING)
    {
        setCoolingFanPower(0);

        return;
    }

    float temperature =
        temperatureManager.getAverageTemperature();

    float hotTemperature =
        temperatureManager.getHotTemperature();

    // -----------------------------------------------------
    // Invalid temperature
    // -----------------------------------------------------

    if (isnan(temperature))
    {
        setCoolingFanPower(0);

        return;
    }

    // =====================================================
    // CRITICAL TEMPERATURE
    // =====================================================

    if (!isnan(hotTemperature) &&
        hotTemperature >= MAX_SAFE_TEMP)
    {
        setCoolingFanPower(100);

        Serial.println(
            "[FAN] CRITICAL TEMP -> COOLING FAN 100%"
        );

        return;
    }

    // =====================================================
    // HIGH TEMPERATURE
    // =====================================================

    /*
     * Cooling fan starts when chamber temperature
     * approaches/exceeds the target.
     *
     * Example:
     *
     * Target = 70 C
     *
     * < 70 C       -> OFF
     * 70-72 C      -> 30%
     * 72-75 C      -> 60%
     * 75-78 C      -> 80%
     * >= 78 C      -> 100%
     */

    float targetTemperature =
        settings.temperature;

    uint8_t newCoolingPower = 0;

    if (temperature >= targetTemperature + 8.0f)
    {
        newCoolingPower = 100;
    }
    else if (temperature >= targetTemperature + 5.0f)
    {
        newCoolingPower = 80;
    }
    else if (temperature >= targetTemperature + 2.0f)
    {
        newCoolingPower = 60;
    }
    else if (temperature >= targetTemperature)
    {
        newCoolingPower = 30;
    }
    else
    {
        newCoolingPower = 0;
    }

    setCoolingFanPower(
        newCoolingPower
    );
    
}
// =====================================================
// POST-DRYING COOLING
// =====================================================

void Application::updatePostDryingCooling()
{
    if (!coolingAfterDrying)
    {
        return;
    }

    float temperature =
        temperatureManager.getAverageTemperature();

    // -------------------------------------------------
    // Invalid temperature
    // -------------------------------------------------

    if (isnan(temperature))
    {
        // Keep cooling for safety.
        setCirculationFan(false);
        setCoolingFanPower(100);

        return;
    }

    // -------------------------------------------------
    // Cooling still required
    // -------------------------------------------------

    if (temperature > COOLING_FINISH_TEMP)
    {
        setCirculationFan(false);
        setCoolingFanPower(100);

        return;
    }

    // -------------------------------------------------
    // Cooling complete
    // -------------------------------------------------

    coolingAfterDrying = false;
    coolingComplete = true;

    setCirculationFan(false);
    setCoolingFanPower(0);

    Serial.println();
    Serial.println(
        "================================");

    Serial.println(
        "[COOLING] COOLING COMPLETE");

    Serial.print(
        "[COOLING] Chamber temperature: ");

    Serial.print(
        temperature);

    Serial.println(" C");

    Serial.println(
        "================================");

    // -------------------------------------------------
    // Completion notification
    // -------------------------------------------------

    buzzer.success();

    state = STATE_FINISHED;

    finishScreenShown = false;

    drawFinishedScreen();
}