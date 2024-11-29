#pragma once

#ifndef _PSXMC__GAME_WORLD_GENERATION__BINARY_GREEDY_MESHER_H_
#define _PSXMC__GAME_WORLD_GENERATION__BINARY_GREEDY_MESHER_H_

#include "../chunk_structure.h"
#include "../../../../util/inttypes.h"
#include "../../../../util/preprocessor.h"
#include "../../../../math/math_utils.h"
#include "../../../blocks/breaking_state.h"
#include "plane_meshing_data.h"

// This implementation is based on TanTan's Binary Greedy Mesher demo:
// https://github.com/TanTanDev/binary_greedy_mesher_demo/blob/main/src/greedy_mesher_optimized.rs

void binaryGreedyMesherBuildMesh(Chunk* chunk, const BreakingState* breaking_state);
// lod_size here should be a value in the interval [0,CHUNK_SIZE]
// where CHUNK_SIZE as a value indicates 1:1 with every block
void binaryGreedyMesherConstructPlane(Chunk* chunk,
                                      PlaneMeshingData* data,
                                      const u32 lod_size);

void binaryGreedyMesherConstructBreakingOverlay(Chunk* chunk, const BreakingState* breaking_state);

#endif // _PSXMC__GAME_WORLD_GENERATION__BINARY_GREEDY_MESHER_H_
