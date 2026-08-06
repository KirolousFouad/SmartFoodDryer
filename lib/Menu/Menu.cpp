/*
-------------------------------------------------------
Smart Food Dryer Firmware
Version : 2.1.0
Module  : Menu
-------------------------------------------------------
*/

#include "Menu.h"

Menu::Menu()
{
    title = "";

    count = 0;
}

void Menu::setTitle(const char* text)
{
    title = text;
}

const char* Menu::getTitle() const
{
    return title;
}

bool Menu::addItem(const char* text,
                   Action action,
                   MenuID next,
                   uint8_t value)
{
    if(count >= MAX_ITEMS)
        return false;

    items[count].text = text;

    items[count].action = action;

    items[count].nextMenu = next;

    items[count].value = value;

    count++;

    return true;
}

uint8_t Menu::size() const
{
    return count;
}

const MenuItem& Menu::get(uint8_t index) const
{
    return items[index];
}