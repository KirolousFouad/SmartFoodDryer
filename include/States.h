/*
-------------------------------------------------------
Smart Food Dryer Firmware
Version : 2.1.0
Module  : States
-------------------------------------------------------
*/

#ifndef STATES_H
#define STATES_H

#include <Arduino.h>

enum State : uint8_t
{
    STATE_IDLE = 0,

    STATE_MENU,

    STATE_EDIT,

    STATE_RUNNING,

    STATE_PAUSED,

    STATE_FINISHED,

    STATE_ERROR
};

enum Event : uint8_t
{
    EVENT_NONE = 0,

    EVENT_LEFT,

    EVENT_RIGHT,

    EVENT_CLICK,

    EVENT_LONG_CLICK
};

enum Action : uint8_t
{
    ACTION_NONE = 0,

    ACTION_OPEN_MENU,

    ACTION_BACK,

    ACTION_START,

    ACTION_EDIT,

    ACTION_SAVE,

    ACTION_CANCEL,

    ACTION_CUSTOM
};

enum MenuID : uint8_t
{
    MENU_MAIN = 0,

    MENU_AUTO,

    MENU_MANUAL,

    MENU_SETTINGS,

    MENU_ABOUT,

    MENU_RECIPE_INFO
};

#endif