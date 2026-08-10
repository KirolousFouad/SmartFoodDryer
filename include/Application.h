#ifndef APPLICATION_H
#define APPLICATION_H

#include "Display.h"
#include "Encoder.h"
#include "Menu.h"
#include "MenuManager.h"
#include "SystemState.h"
#include "DryerSettings.h"
#include "Dryer.h"
#include "TemperatureManager.h"

class Application
{
public:

    Application();

    void begin();
    void update();

private:

    // =====================================================
    // Hardware / Modules
    // =====================================================

    Display display;
    Encoder encoder;
    MenuManager menuManager;

    Dryer dryer;
    TemperatureManager temperatureManager;

    // =====================================================
    // System
    // =====================================================

    SystemState state;
    DryerSettings settings;

    // =====================================================
    // Timers
    // =====================================================

    unsigned long lastHeartbeat;
    unsigned long dryingStartTime;
    unsigned long lastDryerUpdate;
    unsigned long lastDisplayUpdate;
    unsigned long lastTemperatureControl;

    // =====================================================
    // Temperature Control
    // =====================================================

    bool heaterEnabled;

    static constexpr float TEMP_HYSTERESIS = 1.0f;
    static constexpr float MAX_SAFE_TEMP = 90.0f;

    void updateTemperatureControl();

    uint8_t calculateHeaterPower(
        float currentTemperature,
        float targetTemperature
    );

    // =====================================================
    // UI
    // =====================================================

    bool finishScreenShown;

    // =====================================================
    // Menu
    // =====================================================

    void handleMenu(
        EncoderEvent event
    );

    void handleMenuAction(
        MenuAction action,
        uint8_t parameter
    );

    void drawCurrentMenu();

    // =====================================================
    // Manual Temperature
    // =====================================================

    void handleManualTemperature(
        EncoderEvent event
    );

    void drawManualTemperature();

    // =====================================================
    // Manual Weight
    // =====================================================

    void handleManualWeight(
        EncoderEvent event
    );

    void drawManualWeight();

    // =====================================================
    // Ready
    // =====================================================

    void handleReady(
        EncoderEvent event
    );

    void drawReadyScreen();

    // =====================================================
    // Running
    // =====================================================

    void handleRunning(
        EncoderEvent event
    );

    void drawRunningScreen();

    // =====================================================
    // Paused
    // =====================================================

    void handlePaused(
        EncoderEvent event
    );

    void drawPausedScreen();

    // =====================================================
    // Finished
    // =====================================================

    void handleFinished(
        EncoderEvent event
    );

    void drawFinishedScreen();

    // =====================================================
    // Error
    // =====================================================

    void handleError(
        EncoderEvent event
    );

    void drawErrorScreen();
};

#endif
