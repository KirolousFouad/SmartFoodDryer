#ifndef RECIPES_H
#define RECIPES_H

#include <Arduino.h>

// =====================================================
// RECIPE STRUCTURE
// =====================================================

struct Recipe
{
    const char* name;

    uint8_t id;

    // Recommended drying temperature
    uint8_t temperature;

    // Expected final dry weight as percentage
    // of the starting fresh weight.
    //
    // Example:
    // 1000 g starting weight
    // 12% yield
    // -> 120 g target weight
    uint8_t dryYieldPercent;
};

#endif