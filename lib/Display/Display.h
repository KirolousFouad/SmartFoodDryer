#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class Display
{
public:

    Display();

    void begin();

    void clear();

    void print(uint8_t col,
               uint8_t row,
               const char* text);

    void center(uint8_t row,
                const char* text);
    void drawMenu(const char* title, const char* selectedItem);
    void drawTitle(const char* title);
    void drawSelectedItem(const char* item);
private:

    LiquidCrystal_I2C lcd;
};

#endif