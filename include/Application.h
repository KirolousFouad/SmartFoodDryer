#ifndef APPLICATION_H
#define APPLICATION_H

#include <Arduino.h>

#include "Config.h"
#include "Display.h"
#include "Encoder.h"
#include "MenuManager.h"
#include "Dryer.h"
#include "SCRController.h"
#include "TemperatureManager.h"
#include "RecipeDatabase.h"
#include "DryerSettings.h"
#include "WeightSensor.h"

// =====================================================
// SAFETY
// =====================================================

constexpr float MAX_SAFE_TEMP = 120.0f;

// =====================================================
// APPLICATION STATES
// =====================================================

enum ApplicationState
{
    STATE_MENU,

    STATE_MANUAL_TEMP,
    STATE_MANUAL_WEIGHT,

    STATE_READY,

    STATE_RUNNING,
    STATE_PAUSED,

    STATE_FINISHED,
    STATE_ERROR
};

// =====================================================
// APPLICATION
// =====================================================

class Application
{
public:

    Application();

    void begin();
    void update();

private:

    // =================================================
    // HARDWARE / MODULES
    // =================================================

    Display display;

    Encoder encoder;

    MenuManager menuManager;

    Dryer dryer;

    SCRController scr;

    TemperatureManager temperatureManager;

    // =================================================
    // SETTINGS
    // =================================================

    DryerSettings settings;

    // =================================================
    // STATE
    // =================================================

    ApplicationState state;

    WeightSensor weightSensor;

    // =================================================
    // TIMERS
    // =================================================

    unsigned long lastHeartbeat;

    unsigned long dryingStartTime;

    unsigned long lastDryerUpdate;

    unsigned long lastDisplayUpdate;

    unsigned long lastTemperatureControl;

    unsigned long targetReachedStartTime;

    unsigned long lastSCRPulse;

    // =================================================
    // FLAGS
    // =================================================

    bool heaterEnabled;

    bool finishScreenShown;

    bool targetReached;

    // =================================================
    // SCR CONTROL
    // =================================================

    uint8_t currentSoftwarePower;

    uint8_t targetSoftwarePower;

    bool scrResetInProgress;

    uint8_t scrResetStepsRemaining;

    // =================================================
    // MENU
    // =================================================

    void handleMenu(
        EncoderEvent event
    );

    void handleMenuAction(
        MenuAction action,
        uint8_t parameter
    );

    void drawCurrentMenu();

    // =================================================
    // MANUAL TEMPERATURE
    // =================================================

    void handleManualTemperature(
        EncoderEvent event
    );

    void drawManualTemperature();

    // =================================================
    // MANUAL WEIGHT
    // =================================================

    void handleManualWeight(
        EncoderEvent event
    );

    void drawManualWeight();

    // =================================================
    // READY
    // =================================================

    void handleReady(
        EncoderEvent event
    );

    void drawReadyScreen();

    // =================================================
    // RUNNING
    // =================================================

    void handleRunning(
        EncoderEvent event
    );

    void drawRunningScreen();

    // =================================================
    // PAUSED
    // =================================================

    void handlePaused(
        EncoderEvent event
    );

    void drawPausedScreen();

    // =================================================
    // FINISHED
    // =================================================

    void handleFinished(
        EncoderEvent event
    );

    void drawFinishedScreen();

    // =================================================
    // ERROR
    // =================================================

    void handleError(
        EncoderEvent event
    );

    void drawErrorScreen();

    // =================================================
    // TEMPERATURE CONTROL
    // =================================================

    void updateTemperatureControl();

    uint8_t calculateHeaterPower(
        float currentTemperature,
        float targetTemperature
    );

    void checkTargetTemperatureHold();

    // =================================================
    // SCR CONTROL
    // =================================================

    void updateSCRControl();

    void increaseSCRPulse();

    void decreaseSCRPulse();

    void startSCRReset();

    void updateSCRReset();

    void stopSCRMovement();

};

#endif