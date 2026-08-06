#ifndef MENU_H
#define MENU_H

#include <Arduino.h>

#define MAX_MENU_ITEMS 10

class Menu
{
public:

    Menu();

    void setTitle(const char* title);

    bool addItem(const char* item);

    void next();

    void previous();

    uint8_t getSelectedIndex() const;

    const char* getTitle() const;

    const char* getItem(uint8_t index) const;

    uint8_t getItemCount() const;

private:

    const char* menuTitle;

    const char* items[MAX_MENU_ITEMS];

    uint8_t itemCount;

    uint8_t selectedIndex;
};

#endif