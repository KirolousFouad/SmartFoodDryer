#include "Display.h"
#include <string.h>

Display::Display()
    : lcd(0x27, 16, 2)
{
}

void Display::begin()
{
    lcd.init();
    lcd.backlight();
    lcd.clear();
}

void Display::clear()
{
    lcd.clear();
}

void Display::print(uint8_t col,
                    uint8_t row,
                    const char* text)
{
    lcd.setCursor(col, row);
    lcd.print(text);
}

void Display::center(uint8_t row,
                     const char* text)
{
    uint8_t len = strlen(text);

    uint8_t col = 0;

    if (len < 16)
        col = (16 - len) / 2;

    lcd.setCursor(col, row);
    lcd.print(text);
}