/*
-------------------------------------------------------
Smart Food Dryer Firmware
Version : 2.1.0
Module  : Menu
-------------------------------------------------------
*/

#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include "States.h"

#define MAX_ITEMS 10

struct MenuItem
{
    const char* text;

    Action action;

    uint8_t value;

    MenuID nextMenu;
};

class Menu
{
private:

    const char* title;

    MenuItem items[MAX_ITEMS];

    uint8_t count;

public:

    Menu();

    void setTitle(const char* text);

    const char* getTitle() const;

    bool addItem(const char* text,
                 Action action,
                 MenuID next,
                 uint8_t value = 0);

    uint8_t size() const;

    const MenuItem& get(uint8_t index) const;

};

#endif