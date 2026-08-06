#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include "MenuAction.h"

#define MAX_MENU_ITEMS 10

struct MenuItem
{
    const char* text;
    MenuAction action;
    uint8_t parameter;
};

class Menu
{
public:

    Menu();

    // Menu configuration
    void setTitle(const char* title);

    bool addItem(
        const char* text,
        MenuAction action,
        uint8_t parameter = 0
    );

    // Navigation
    void next();
    void previous();
    void resetSelection();

    // Getters
    uint8_t getSelectedIndex() const;

    const char* getTitle() const;

    const char* getItem(uint8_t index) const;

    MenuAction getSelectedAction() const;

    uint8_t getSelectedParameter() const;

    uint8_t getItemCount() const;

private:

    const char* menuTitle;

    MenuItem items[MAX_MENU_ITEMS];

    uint8_t itemCount;

    uint8_t selectedIndex;
};

#endif