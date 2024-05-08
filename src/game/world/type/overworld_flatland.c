#include "overworld_flatland.h"

#include <interface99_extensions.h>
#include <stdlib.h>

#include "../chunk/chunk_structure.h"
#include "../generation/noise.h"
#include "../../blocks/blocks.h"

// ==== GENERATOR ====

// Forward declaration
void chunkInit(Chunk *chunk);
void chunkDestroy(const Chunk* chunk);

void overworldFlatlandChunkGeneratorInit(VSelf) __attribute__((alias("OverworldFlatlandChunkGenerator_init")));
void OverworldFlatlandChunkGenerator_init(VSelf) {
    // Do nothing
}

void overworldFlatlandChunkGeneratorDestroy(VSelf) __attribute__((alias("OverworldFlatlandChunkGenerator_destroy")));
void OverworldFlatlandChunkGenerator_destroy(VSelf) {
    // Do nothing
}

void overworldFlatlandGeneneratorGenerate(VSelf, Chunk* chunk) __attribute__((alias("OverworldFlatlandChunkGenerator_generate")));
void OverworldFlatlandChunkGenerator_generate(VSelf, Chunk* chunk) {
    const VECTOR* position = &chunk->position;
    for (i32 x = 0; x < CHUNK_SIZE; x++) {
        for (i32 z = 0; z < CHUNK_SIZE; z++) {
            for (i32 y = 0; y < CHUNK_SIZE; y++) {
                if (y < 2) {
                    chunk->blocks[chunkBlockIndex(x, y, z)] = stoneBlockCreate();
                    // chunk->blocks[chunkBlockIndex(x, y, z)] = airBlockCreate();
                } else if (y < 4) {
                    chunk->blocks[chunkBlockIndex(x, y, z)] = dirtBlockCreate();
                    // chunk->blocks[chunkBlockIndex(x, y, z)] = airBlockCreate();
                } else if (y == 4) {
                    chunk->blocks[chunkBlockIndex(x, y, z)] = grassBlockCreate();
                } else {
                    if (y == 5 && x % 3 == 0 && z % 3 == 0) {
                        chunk->blocks[chunkBlockIndex(x, y, z)] = grassBlockCreate();
                        // chunk->blocks[chunkBlockIndex(x, y, z)] = airBlockCreate();
                    } else {
                        chunk->blocks[chunkBlockIndex(x, y, z)] = airBlockCreate();
                    }
                }
            }
        }
    }
}

// ==== PROVIDER ====

void overworldFlatlandChunkProviderInit(VSelf) __attribute__((alias("OverworldFlatlandChunkProvider_init")));
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

Chunk* overworldFlatlandProvideChunk(VSelf, const VECTOR position) __attribute__((alias("OverworldFlatlandChunkProvider_provide")));
Chunk* OverworldFlatlandChunkProvider_provide(VSelf, const VECTOR position) {
    VSELF(OverworldFlatlandChunkProvider);
    Chunk* chunk = malloc(sizeof(Chunk));
    assert(chunk != NULL);
    chunk->position = position;
    chunkInit(chunk);
    VCALL(self->generator, generate, chunk);
    return chunk;
}

bool overworldFlatlandSaveChunk(VSelf, Chunk* chunk) __attribute__((alias("OverworldFlatlandChunkProvider_save")));
bool OverworldFlatlandChunkProvider_save(VSelf, Chunk* chunk) {
    // TODO: Implement this when world saving is possible
    chunkDestroy(chunk);
    return true;
}