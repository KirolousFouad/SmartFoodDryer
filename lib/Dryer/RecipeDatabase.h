#ifndef RECIPE_DATABASE_H
#define RECIPE_DATABASE_H

#include "Recipes.h"

#define RECIPE_COUNT 5

class RecipeDatabase
{
public:

    static const Recipe& getRecipe(uint8_t id);
};

#endif