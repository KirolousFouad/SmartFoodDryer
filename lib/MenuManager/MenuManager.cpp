#include "MenuManager.h"

MenuManager::MenuManager()
{
    current = &mainMenu;
}

void MenuManager::begin()
{
    //-------------------------
    // Main Menu
    //-------------------------

    mainMenu.setTitle("Food Dryer");

    mainMenu.addItem(
        "Auto",
        ACTION_OPEN_RECIPES
    );

    mainMenu.addItem(
        "Manual",
        ACTION_OPEN_MANUAL
    );

    //-------------------------
    // Recipe Menu
    //-------------------------

    recipeMenu.setTitle("Recipes");

    recipeMenu.addItem(
        "Mango",
        ACTION_START_RECIPE,
        0
    );

    recipeMenu.addItem(
        "Banana",
        ACTION_START_RECIPE,
        1
    );

    recipeMenu.addItem(
        "Apple",
        ACTION_START_RECIPE,
        2
    );

    recipeMenu.addItem(
        "Orange",
        ACTION_START_RECIPE,
        3
    );

    recipeMenu.addItem(
        "Strawberry",
        ACTION_START_RECIPE,
        4
    );

    current = &mainMenu;
}

Menu* MenuManager::currentMenu()
{
    return current;
}

void MenuManager::openMain()
{
    current = &mainMenu;
    current->resetSelection();
}

void MenuManager::openRecipes()
{
    current = &recipeMenu;
    current->resetSelection();
}