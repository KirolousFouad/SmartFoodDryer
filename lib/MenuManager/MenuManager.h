#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include "Menu.h"

class MenuManager
{
public:

    MenuManager();

    void begin();

    Menu* currentMenu();

    void openMain();

    void openRecipes();

private:

    Menu mainMenu;

    Menu recipeMenu;

    Menu* current;
};

#endif