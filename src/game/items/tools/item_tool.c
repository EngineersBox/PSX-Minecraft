#include "item_tool.h"

const fixedi32 ITEM_MATERIAL_SPEED_MULTIPLIER[ITEM_MATERIAL_COUNT] = {
    [ITEMMATERIAL_NONE] = 1 << FIXED_POINT_SHIFT,
    [ITEMMATERIAL_WOOD] = 2 << FIXED_POINT_SHIFT,
    [ITEMMATERIAL_STONE] = 4 << FIXED_POINT_SHIFT,
    [ITEMMATERIAL_IRON] = 6 << FIXED_POINT_SHIFT,
    [ITEMMATERIAL_GOLD] = 12 << FIXED_POINT_SHIFT,
    [ITEMMATERIAL_DIAMOND] = 8 << FIXED_POINT_SHIFT
};