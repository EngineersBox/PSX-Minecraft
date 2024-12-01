#pragma once

#ifndef PSXMC_BLOCKS_H
#define PSXMC_BLOCKS_H

#include "../../util/inttypes.h"
#include "block_id.h"
#include "block.h"
#include "../items/tools/tool_type.h"
// These includes allow for only including blocks.h
// and getting access to all blocks instantly
#include "block_air.h"
#include "block_stone.h"
#include "block_grass.h"
#include "block_dirt.h"
#include "block_cobblestone.h"

#define BLOCK_COUNT 59

extern BlockAttributes block_attributes[BLOCK_COUNT];
extern BlockConstructor block_constructors[BLOCK_COUNT];

#define blockEqual(b0, b1) ((b0)->id == (b1)->id && (b0)->metadata_id == (b1)->metadata_id)
#define blockGetAttribute(id, attr) (block_attributes[(id)].attr)
#define blockGetName(id) blockGetAttribute(id, name)
#define blockGetType(id) blockGetAttribute(id, type)
#define blockGetSlipperiness(id) blockGetAttribute(id, slipperiness)
#define blockGetHardness(id) blockGetAttribute(id, hardness)
#define blockGetResistance(id) blockGetAttribute(id, resistance)
#define blockGetToolType(id) blockGetAttribute(id, tool_type)
#define blockGetToolMaterial(id) blockGetAttribute(id, tool_material)
#define blockGetCanHarvestBitSet(id) blockGetAttribute(id, can_harvest)
#define blockGetItemCanHarvest(id, tool_type) ((blockGetCanHarvestBitSet(id) >> (tool_type)) & 0b1)
#define blockCanPropagateSunlight(id) blockGetAttribute(id, propagates_sunlight)
#define blockCanPropagateBlocklight(id) blockGetAttribute(id, propagates_blocklight)
#define blockGetFaceAttributes(id, metadata_id) (&blockGetAttribute(id, face_attributes)[(metadata_id) * FACE_DIRECTION_COUNT])

bool blockCanHarvest(ToolType block_tool_type,
                     ItemMaterial block_tool_material,
                     ToolType item_tool_type,
                     ItemMaterial item_tool_material,
                     const Block* block);

void blocksInitialiseBuiltin();

#endif // PSXMC_BLOCKS_H
