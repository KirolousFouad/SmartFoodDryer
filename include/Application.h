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
#include "Buzzer.h"
#include "MenuAction.h"

// =====================================================
// SAFETY
// =====================================================

constexpr float MAX_SAFE_TEMP = 120.0f;

// =====================================================
// APPLICATION STATES
// =====================================================

enum ApplicationState
{
    STATE_TARE_CONFIRM,
    STATE_TARING,

    STATE_MENU,

    STATE_MANUAL_TEMP,
    STATE_MANUAL_WEIGHT,

    STATE_READY,

    STATE_RUNNING,
    STATE_PAUSED,

    STATE_FINISHED,
    STATE_ERROR
};

enum ApplicationError
{
    ERROR_NONE,

    ERROR_TEMP_SENSOR,
    ERROR_OVER_TEMPERATURE,
    ERROR_WEIGHT_SENSOR,
    ERROR_INVALID_TARGET_WEIGHT,
    ERROR_INVALID_STARTING_WEIGHT,
    ERROR_TARGET_WEIGHT_INVALID
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

    Buzzer buzzer;

    // =================================================
    // SETTINGS
    // =================================================

    DryerSettings settings;

    // =================================================
    // STATE
    // =================================================

    ApplicationState state;

    ApplicationError error;

    WeightSensor weightSensor;

    // =================================================
    // POST-DRYING COOLING
    // =================================================

    bool coolingAfterDrying;

    bool coolingComplete;
    // =================================================
    // DRYING WEIGHT CONTROL
    // =================================================

    float startingWeight;
    float currentWeight;
    float targetWeight;

    unsigned long weightTargetStartTime;

    bool weightTargetReached;
    bool startupTareScreenShown;
    bool startupTareRequested;
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
    // PAUSE / RESUME
    // =================================================

    bool pauseResumePending;
    // =========================================================
    // FANS
    // =========================================================

    bool circulationFanOn;

    uint8_t coolingFanPower;

    unsigned long lastCoolingFanUpdate;

    // =================================================
    // MENU
    // =================================================

    void handleMenu(
        EncoderEvent event);

    void handleMenuAction(
        MenuAction action,
        uint8_t parameter);

    void drawCurrentMenu();

    // =================================================
    // MANUAL TEMPERATURE
    // =================================================

    void handleManualTemperature(
        EncoderEvent event);

    void drawManualTemperature();

    // =================================================
    // MANUAL WEIGHT
    // =================================================

    void handleManualWeight(
        EncoderEvent event);

    void drawManualWeight();

    // =================================================
    // READY
    // =================================================

    void handleReady(
        EncoderEvent event);

    void drawReadyScreen();

    // =================================================
    // RUNNING
    // =================================================

    void handleRunning(
        EncoderEvent event);

    void drawRunningScreen();

    // =================================================
    // PAUSED
    // =================================================

    void handlePaused(
        EncoderEvent event);

    void drawPausedScreen();

    // =================================================
    // FINISHED
    // =================================================

    void handleFinished(
        EncoderEvent event);

    void drawFinishedScreen();

    // =================================================
    // ERROR
    // =================================================

    void handleError(
        EncoderEvent event);

    void drawErrorScreen();

    // =================================================
    // TEMPERATURE CONTROL
    // =================================================

    void updateTemperatureControl();

    uint8_t calculateHeaterPower(
        float currentTemperature,
        float targetTemperature);

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
    void updateWeightControl();

    void captureStartingWeight();

    void checkTargetWeight();

    void finishDrying();
    // =========================================================
    // FAN CONTROL
    // =========================================================

    void updateFanControl();

    void setCirculationFan(bool on);

    void setCoolingFanPower(uint8_t power);
    void updatePostDryingCooling();
    const char* getErrorText() const;

    void handleTareConfirm(
        EncoderEvent event);

    void handleTaring();

    void drawTareConfirmScreen();
    void handleStartupTare(
        EncoderEvent event);

    void drawStartupTare();
    void drawTaringScreen();
    void returnToMainMenu();
};

#endif