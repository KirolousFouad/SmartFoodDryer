#include "Display.h"
#include "Recipes.h"

extern LiquidCrystal_I2C lcd;

Display display(&lcd);

Display::Display(LiquidCrystal_I2C* displayLCD)
{
    lcd = displayLCD;
}

void Display::begin()
{
    lcd->init();
    lcd->backlight();
}

void Display::clear()
{
    lcd->clear();
}

void Display::showMainMenu(byte selected)
{
    lcd->clear();

    lcd->setCursor(0,0);
    lcd->print("Food Dryer");

    lcd->setCursor(0,1);

    if(selected==0)
        lcd->print("> Auto");
    else
        lcd->print("> Manual");
}

void Display::showAutoMenu(byte selected)
{
    lcd->clear();

    lcd->print("Auto Mode");

    lcd->setCursor(0,1);

    lcd->print("> ");
    lcd->print(recipes[selected].name);
}

void Display::showRecipe(byte recipeIndex)
{
    lcd->clear();

    lcd->print(recipes[recipeIndex].name);

    lcd->setCursor(0,1);

    lcd->print(recipes[recipeIndex].temperature);
    lcd->print((char)223);
    lcd->print("C ");

    lcd->print(recipes[recipeIndex].finalPercent);
    lcd->print("%");
}

void Display::showManualTemp(int temp)
{
    lcd->clear();

    lcd->print("Set Temp");

    lcd->setCursor(0,1);

    lcd->print(temp);
    lcd->print((char)223);
    lcd->print("C");
}

void Display::showManualWeight(int weight)
{
    lcd->clear();

    lcd->print("Set Weight");

    lcd->setCursor(0,1);

    lcd->print(weight);
    lcd->print(" g");
}

void Display::showReady()
{
    lcd->clear();

    lcd->print("Ready?");

    lcd->setCursor(0,1);

    lcd->print("> Press Btn");
}

void Display::showDrying(int temp,int weight,int minutes)
{
    lcd->clear();

    lcd->print(temp);
    lcd->print((char)223);
    lcd->print("C ");

    lcd->print(weight);
    lcd->print("g");

    lcd->setCursor(0,1);

    lcd->print("Time ");

    lcd->print(minutes);

    lcd->print("m");
}