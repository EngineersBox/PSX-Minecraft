#pragma once

#ifndef _PSX_MINECRAFT__GAME_ITEMS_TOOLS__TOOL_TYPE_H_
#define _PSX_MINECRAFT__GAME_ITEMS_TOOLS__TOOL_TYPE_H_

#define TOOL_TYPE_COUNT 7
#define TOOL_TYPE_COUNT_BITS 3
typedef enum ToolType {
    TOOLTYPE_NONE = 0,
    TOOLTYPE_FISHING_ROD,
    TOOLTYPE_PICKAXE,
    TOOLTYPE_AXE,
    TOOLTYPE_SWORD,
    TOOLTYPE_SHOVEL,
    TOOLTYPE_HOE
} ToolType;

#define toolTypeBitset(none, pickaxe, axe, sword, shovel, hoe) (\
      ((none) << TOOLTYPE_NONE) \
    | ((pickaxe) << TOOLTYPE_PICKAXE) \
    | ((axe) << TOOLTYPE_AXE) \
    | ((sword) << TOOLTYPE_SWORD) \
    | ((shovel) << TOOLTYPE_SHOVEL) \
    | ((hoe) << TOOLTYPE_HOE) \
)

#endif // _PSX_MINECRAFT__GAME_ITEMS_TOOLS__TOOL_TYPE_H_