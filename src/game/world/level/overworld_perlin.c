#include "overworld_perlin.h"

#include <stdlib.h>

#include "../../../util/interface99_extensions.h"
#include "../chunk/chunk_structure.h"
#include "../generation/noise.h"
#include "../../blocks/blocks.h"

// ==== GENERATOR ====

// Forward declaration
void chunkInit(Chunk *chunk);
void chunkDestroy(const Chunk* chunk);

void overworldPerlinChunkGeneratorInit(VSelf) ALIAS("OverworldPerlinChunkGenerator_init");
void OverworldPerlinChunkGenerator_init(VSelf) {
    // Do nothing
}

void overworldPerlinChunkGeneratorDestroy(VSelf) ALIAS("OverworldPerlinChunkGenerator_destroy");
void OverworldPerlinChunkGenerator_destroy(VSelf) {
    // Do nothing
}

void overworldPerlinGeneneratorGenerate(VSelf, Chunk* chunk, ChunkHeightmap* heightmap) ALIAS("OverworldPerlinChunkGenerator_generate");
void OverworldPerlinChunkGenerator_generate(VSelf, Chunk* chunk, ChunkHeightmap* heightmap) {
    const VECTOR* position = &chunk->position;
    for (i32 x = 0; x < CHUNK_SIZE; x++) {
        for (i32 z = 0; z < CHUNK_SIZE; z++) {
            const i32 xPos = x + (position->vx * CHUNK_SIZE);
            const i32 zPos = z + (position->vz * CHUNK_SIZE);
            // FMath::Clamp(
            //     FMath::RoundToInt(
            //         (Noise->GetNoise(Xpos, Ypos) + 1) * Size / 2
            //     ),
            //     0,
            //     Size
            // );
            const int height = noise2d(xPos * ONE, zPos * ONE) >> 1;
            // clamp(
            //     noise,
            //     0,
            //     CHUNK_SIZE
            // );
            for (i32 y = 0; y < CHUNK_SIZE; y++) {
                const i32 world_y = (position->vy * CHUNK_SIZE) + y;
                const i32 offset_world_y = world_y + (CHUNK_SIZE * 6); // !IMPORTANT: TESTING OFFSET
                IBlock* iblock;
                if (offset_world_y < height - 3) {
                    iblock = chunk->blocks[chunkBlockIndex(x, y, z)] = stoneBlockCreate(NULL);
                } else if (offset_world_y < height - 1) {
                    iblock = chunk->blocks[chunkBlockIndex(x, y, z)] = dirtBlockCreate(NULL);
                } else if (offset_world_y == height - 1) {
                    iblock = chunk->blocks[chunkBlockIndex(x, y, z)] = grassBlockCreate(NULL);
                } else {
                    iblock = chunk->blocks[chunkBlockIndex(x, y, z)] = airBlockCreate(NULL);
                }
                const Block* block = VCAST_PTR(Block*, iblock);
                if (block->light_level > 0) {
                    const VECTOR position = vec3_i32(x, y, z);
                    chunkSetLightValue(
                        chunk,
                        &position,
                        block->light_level,
                        LIGHT_TYPE_BLOCK
                    );
                }
                // Update heightmap
                if (block->id != BLOCKID_AIR) {
                    const u32 index = chunkHeightmapIndex(x, z);
                    const u32 top = (*heightmap)[index];
                    (*heightmap)[index] = max(top, (u32) world_y);
                }
            }
        }
    }
}

// ==== PROVIDER ====

void overworldPerlinChunkProviderInit(VSelf) ALIAS("OverworldPerlinChunkProvider_init");
void OverworldPerlinChunkProvider_init(VSelf) {
    VSELF(OverworldPerlinChunkProvider);
    DYN_PTR(
        &self->generator,
        OverworldPerlinChunkGenerator,
        IChunkGenerator,
        malloc(sizeof(OverworldPerlinChunkGenerator))
    );
    VCALL(self->generator, init);
}

void overworldPerlinChunkProviderDestroy(VSelf) ALIAS("OverworldPerlinChunkProvider_destroy");
void OverworldPerlinChunkProvider_destroy(VSelf) {
    VSELF(OverworldPerlinChunkProvider);
    VCALL(self->generator, destroy);
    free(self->generator.self);
}

Chunk* overworldPerlinProvideChunk(VSelf, const VECTOR position, ChunkHeightmap* heightmap) ALIAS("OverworldPerlinChunkProvider_provide");
Chunk* OverworldPerlinChunkProvider_provide(VSelf, const VECTOR position, ChunkHeightmap* heightmap) {
    VSELF(OverworldPerlinChunkProvider);
    Chunk* chunk = malloc(sizeof(Chunk));
    assert(chunk != NULL);
    chunk->position = position;
    chunkInit(chunk);
    VCALL(self->generator, generate, chunk, heightmap);
    return chunk;
}

bool overworldPerlinSaveChunk(VSelf, Chunk* chunk) ALIAS("OverworldPerlinChunkProvider_save");
bool OverworldPerlinChunkProvider_save(VSelf, Chunk* chunk) {
    // TODO: Implement this when world saving is possible
    chunkDestroy(chunk);
    return true;
}
