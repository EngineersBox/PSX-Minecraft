#include "overworld_flatland.h"

#include <debug.h>
#include <interface99_extensions.h>
#include "../../../core/std/stdlib.h"

#include "../chunk/chunk_structure.h"

// ==== GENERATOR ====

// Forward declaration
void chunkInit(Chunk *chunk);
void chunkDestroy(const Chunk* chunk);

void overworldFlatlandChunkGeneratorInit(VSelf) ALIAS("OverworldFlatlandChunkGenerator_init");
void OverworldFlatlandChunkGenerator_init(UNUSED VSelf) {
    // Do nothing
}

void overworldFlatlandChunkGeneratorDestroy(VSelf) ALIAS("OverworldFlatlandChunkGenerator_destroy");
void OverworldFlatlandChunkGenerator_destroy(UNUSED VSelf) {
    // Do nothing
}

void overworldFlatlandGeneneratorGenerate(VSelf, Chunk* chunk, ChunkHeightmap* heightmap) ALIAS("OverworldFlatlandChunkGenerator_generate");
void OverworldFlatlandChunkGenerator_generate(UNUSED VSelf, Chunk* chunk, ChunkHeightmap* heightmap) {
    for (i32 x = 0; x < CHUNK_SIZE; x++) {
        for (i32 z = 0; z < CHUNK_SIZE; z++) {
            for (i32 y = 0; y < CHUNK_SIZE; y++) {
                const u32 world_y = (chunk->position.vy * CHUNK_SIZE) + y;
                IBlock* iblock;
                if (y < 2) {
                    iblock = chunk->blocks[chunkBlockIndex(x, y, z)] = stoneBlockCreate(NULL,0 );
                    // chunk->blocks[chunkBlockIndex(x, y, z)] = airBlockCreate(NULL);
                } else if (y < 4) {
                    iblock = chunk->blocks[chunkBlockIndex(x, y, z)] = dirtBlockCreate(NULL, 0);
                    // chunk->blocks[chunkBlockIndex(x, y, z)] = airBlockCreate(NULL);
                } else if (y == 4) {
                    iblock = chunk->blocks[chunkBlockIndex(x, y, z)] = grassBlockCreate(NULL, 0);
                } else {
                    if (y == 5 && x % 3 == 0 && z % 3 == 0) {
                        iblock = chunk->blocks[chunkBlockIndex(x, y, z)] = grassBlockCreate(NULL, 0);
                        // chunk->blocks[chunkBlockIndex(x, y, z)] = airBlockCreate(NULL);
                    } else {
                        iblock = chunk->blocks[chunkBlockIndex(x, y, z)] = airBlockCreate(NULL, 0);
                    }
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
                    (*heightmap)[index] = max(top, world_y);
                }
            }
        }
    }
}

// ==== PROVIDER ====

void overworldFlatlandChunkProviderInit(VSelf) ALIAS("OverworldFlatlandChunkProvider_init");
void OverworldFlatlandChunkProvider_init(VSelf) {
    VSELF(OverworldFlatlandChunkProvider);
    DYN_PTR(
        &self->generator,
        OverworldFlatlandChunkGenerator,
        IChunkGenerator,
        malloc(sizeof(OverworldFlatlandChunkGenerator))
    );
    VCALL(self->generator, init);
}

void overworldFlatlandChunkProviderDestroy(VSelf);
void OverworldFlatlandChunkProvider_destroy(VSelf) {
    VSELF(OverworldFlatlandChunkProvider);
    VCALL(self->generator, destroy);
    free(self->generator.self);
}

Chunk* overworldFlatlandProvideChunk(VSelf, const VECTOR position, ChunkHeightmap* heightmap) ALIAS("OverworldFlatlandChunkProvider_provide");
Chunk* OverworldFlatlandChunkProvider_provide(VSelf, const VECTOR position, ChunkHeightmap* heightmap) {
    VSELF(OverworldFlatlandChunkProvider);
    Chunk* chunk = chunkNew();
    chunk->position = position;
    chunkInit(chunk);
    VCALL(self->generator, generate, chunk, heightmap);
    return chunk;
}

bool overworldFlatlandSaveChunk(VSelf, Chunk* chunk) ALIAS("OverworldFlatlandChunkProvider_save");
bool OverworldFlatlandChunkProvider_save(UNUSED VSelf, Chunk* chunk) {
    // TODO: Implement this when world saving is possible
    chunkDestroy(chunk);
    return true;
}
