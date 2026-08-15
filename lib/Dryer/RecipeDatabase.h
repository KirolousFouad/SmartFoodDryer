#ifndef RECIPE_DATABASE_H
#define RECIPE_DATABASE_H

#include "Recipes.h"

class RecipeDatabase
{
public:

    static const Recipe& getRecipe(
        uint8_t recipeID
    );

    static uint8_t getRecipeCount();
};

#endif