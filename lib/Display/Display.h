#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class Display
{
private:
    LiquidCrystal_I2C* lcd;

public:
    Display(LiquidCrystal_I2C* display);

    void begin();

    void clear();

    void showMainMenu(byte selected);

    void showAutoMenu(byte selected);

    void showRecipe(byte recipeIndex);

    void showManualTemp(int temp);

    void showManualWeight(int weight);

    void showReady();

    void showDrying(int temp, int weight, int minutes);
};

extern Display display;

#endif