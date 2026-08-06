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

bool Menu::addItem(
    const char* text,
    MenuAction action,
    uint8_t parameter)
{
    if (itemCount >= MAX_MENU_ITEMS)
        return false;

    items[itemCount].text = text;
    items[itemCount].action = action;
    items[itemCount].parameter = parameter;

    itemCount++;

    return true;
}

void Menu::next()
{
    if (itemCount == 0)
        return;

    selectedIndex++;

    if (selectedIndex >= itemCount)
        selectedIndex = 0;
}

void Menu::previous()
{
    if (itemCount == 0)
        return;

    if (selectedIndex == 0)
        selectedIndex = itemCount - 1;
    else
        selectedIndex--;
}

void Menu::resetSelection()
{
    selectedIndex = 0;
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
    if (index >= itemCount)
        return "";

    return items[index].text;
}

MenuAction Menu::getSelectedAction() const
{
    if (itemCount == 0)
        return ACTION_NONE;

    return items[selectedIndex].action;
}

uint8_t Menu::getSelectedParameter() const
{
    if (itemCount == 0)
        return 0;

    return items[selectedIndex].parameter;
}

uint8_t Menu::getItemCount() const
{
    return itemCount;
}