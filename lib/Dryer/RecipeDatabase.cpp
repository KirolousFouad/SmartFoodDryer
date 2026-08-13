#include "Recipes.h"
#include "RecipeDatabase.h"

static const Recipe recipes[RECIPE_COUNT] =
{
    {
        "Mango",
        0,
        60,
        300
    },

    {
        "Banana",
        1,
        65,
        250
    },

    {
        "Apple",
        2,
        60,
        250
    },

    {
        "Orange",
        3,
        60,
        300
    },

    {
        "Strawberry",
        4,
        55,
        200
    }
};

const Recipe& RecipeDatabase::getRecipe(uint8_t id)
{
    if (id >= RECIPE_COUNT)
    {
        return recipes[0];
    }

    return recipes[id];
}