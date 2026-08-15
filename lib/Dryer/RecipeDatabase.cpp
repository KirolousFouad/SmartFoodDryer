#include "RecipeDatabase.h"

// =====================================================
// RECIPE DATABASE
// =====================================================
//
// dryYieldPercent means:
//
// final dry weight
// ---------------- × 100
// starting fresh weight
//
// These values are recommended starting values.
// Actual yield depends on fruit variety, maturity,
// slice thickness, pretreatment, dryer airflow,
// temperature and final moisture content.
// =====================================================

static const Recipe recipes[] =
{
    // -------------------------------------------------
    // APPLE
    // -------------------------------------------------

    {
        "Apple",
        0,
        60,
        12
    },

    // -------------------------------------------------
    // BANANA
    // -------------------------------------------------

    {
        "Banana",
        1,
        60,
        22
    },

    // -------------------------------------------------
    // MANGO
    // -------------------------------------------------

    {
        "Mango",
        2,
        60,
        18
    },

    // -------------------------------------------------
    // STRAWBERRY
    // -------------------------------------------------

    {
        "Strawberry",
        3,
        55,
        10
    },

    // -------------------------------------------------
    // PINEAPPLE
    // -------------------------------------------------

    {
        "Pineapple",
        4,
        55,
        12
    }
};

// =====================================================
// RECIPE COUNT
// =====================================================

static const uint8_t RECIPE_COUNT =
    sizeof(recipes) / sizeof(recipes[0]);

// =====================================================
// GET RECIPE
// =====================================================

const Recipe& RecipeDatabase::getRecipe(
    uint8_t recipeID
)
{
    if (recipeID >= RECIPE_COUNT)
    {
        return recipes[0];
    }

    return recipes[recipeID];
}

// =====================================================
// GET RECIPE COUNT
// =====================================================

uint8_t RecipeDatabase::getRecipeCount()
{
    return RECIPE_COUNT;
}