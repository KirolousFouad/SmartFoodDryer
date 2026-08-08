#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include "Menu.h"

class MenuManager
{
public:

    MenuManager();

    void begin();

    Menu* currentMenu();

    void openMain();

    void openRecipes();

    void openConfirmation();

    void back();

private:

    Menu mainMenu;

    Menu recipeMenu;

    Menu confirmationMenu;

    Menu* current;
};

#endif