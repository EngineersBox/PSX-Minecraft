#pragma once

#ifndef PSX_MINECRAFT_BLOCKS_H
#define PSX_MINECRAFT_BLOCKS_H

#include "../../util/inttypes.h"
#include "block_id.h"
#include "block.h"
// These includes allow for only including blocks.h
// and getting access to all blocks instantly
#include "block_air.h"
#include "block_stone.h"
#include "block_dirt.h"
#include "block_grass.h"

#define BLOCK_COUNT 5

extern BlockAttributes block_attributes[BLOCK_COUNT];

void blocksInitialiseBuiltin();

#endif // PSX_MINECRAFT_BLOCKS_H
