#pragma once

#ifndef _PSXMC__GAME_ITEMS_TOOLS__TOOL_TYPE_H_
#define _PSXMC__GAME_ITEMS_TOOLS__TOOL_TYPE_H_

#define TOOL_TYPE_COUNT 7
#define TOOL_TYPE_COUNT_BITS 3
typedef enum ToolType {
    TOOL_TYPE_NONE = 0,
    TOOL_TYPE_FISHING_ROD,
    TOOL_TYPE_PICKAXE,
    TOOL_TYPE_AXE,
    TOOL_TYPE_SWORD,
    TOOL_TYPE_SHOVEL,
    TOOL_TYPE_HOE
} ToolType;

#define toolTypeBitset(none, pickaxe, axe, sword, shovel, hoe) ((u8) (\
      ((none) << TOOL_TYPE_NONE) \
    | ((pickaxe) << TOOL_TYPE_PICKAXE) \
    | ((axe) << TOOL_TYPE_AXE) \
    | ((sword) << TOOL_TYPE_SWORD) \
    | ((shovel) << TOOL_TYPE_SHOVEL) \
    | ((hoe) << TOOL_TYPE_HOE) \
))

#endif // _PSXMC__GAME_ITEMS_TOOLS__TOOL_TYPE_H_
