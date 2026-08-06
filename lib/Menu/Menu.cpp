#include "Menu.h"

Menu::Menu()
{
    menuTitle = "";
    itemCount = 0;
    selectedIndex = 0;
}

void Menu::setTitle(const char* title)
{
    menuTitle = title;
}

bool Menu::addItem(const char* item)
{
    if(itemCount >= MAX_MENU_ITEMS)
        return false;

    items[itemCount++] = item;

    return true;
}

void Menu::next()
{
    if(itemCount == 0)
        return;

    selectedIndex++;

    if(selectedIndex >= itemCount)
        selectedIndex = 0;
}

void Menu::previous()
{
    if(itemCount == 0)
        return;

    if(selectedIndex == 0)
        selectedIndex = itemCount - 1;
    else
        selectedIndex--;
}

uint8_t Menu::getSelectedIndex() const
{
    return selectedIndex;
}

const char* Menu::getTitle() const
{
    return menuTitle;
}

const char* Menu::getItem(uint8_t index) const
{
    if(index >= itemCount)
        return "";

    return items[index];
}

uint8_t Menu::getItemCount() const
{
    return itemCount;
}