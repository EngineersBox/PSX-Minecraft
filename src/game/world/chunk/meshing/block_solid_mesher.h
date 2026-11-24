#pragma once

#ifndef _GAME_WORLD_CHUNK_MESHING__BLOCK_SOLID_MESHER_H_
#define _GAME_WORLD_CHUNK_MESHING__BLOCK_SOLID_MESHER_H_

#include "plane_meshing_data.h"
#include "../chunk_structure.h"

void meshBlockTypeSolid(Chunk* chunk,
                        const PlaneMeshingDataKey* data,
                        const Texture* texture_override,
                        const TextureAttributes* texture_attributes_override,
                        const u32 x,
                        const u32 y,
                        const u32 w,
                        const u32 h);

#endif // _GAME_WORLD_CHUNK_MESHING__BLOCK_SOLID_MESHER_H_
