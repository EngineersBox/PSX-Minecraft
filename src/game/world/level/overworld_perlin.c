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

void overworldPerlinGeneneratorGenerate(VSelf, Chunk* chunk) ALIAS("OverworldPerlinChunkGenerator_generate");
void OverworldPerlinChunkGenerator_generate(VSelf, Chunk* chunk) {
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
            const int height = noise2d(xPos * ONE, zPos * ONE) >> 1; // Div by 2
            // clamp(
            //     noise,
            //     0,
            //     CHUNK_SIZE
            // );
            for (i32 y = 0; y < CHUNK_SIZE; y++) {
                const i32 worldY = (position->vy * CHUNK_SIZE) + y + (CHUNK_SIZE * 6); // !IMPORTANT: TESTING OFFSET
                if (worldY < height - 3) {
                    chunk->blocks[chunkBlockIndex(x, y, z)] = stoneBlockCreate(NULL);
                } else if (worldY < height - 1) {
                    chunk->blocks[chunkBlockIndex(x, y, z)] = cobblestoneBlockCreate(NULL);
                } else if (worldY == height - 1) {
                    chunk->blocks[chunkBlockIndex(x, y, z)] = grassBlockCreate(NULL);
                } else {
                    chunk->blocks[chunkBlockIndex(x, y, z)] = airBlockCreate(NULL);
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

Chunk* overworldPerlinProvideChunk(VSelf, const VECTOR position) ALIAS("OverworldPerlinChunkProvider_provide");
Chunk* OverworldPerlinChunkProvider_provide(VSelf, const VECTOR position) {
    VSELF(OverworldPerlinChunkProvider);
    Chunk* chunk = malloc(sizeof(Chunk));
    assert(chunk != NULL);
    chunk->position = position;
    chunkInit(chunk);
    VCALL(self->generator, generate, chunk);
    return chunk;
}

bool overworldPerlinSaveChunk(VSelf, Chunk* chunk) ALIAS("OverworldPerlinChunkProvider_save");
bool OverworldPerlinChunkProvider_save(VSelf, Chunk* chunk) {
    // TODO: Implement this when world saving is possible
    chunkDestroy(chunk);
    return true;
}
