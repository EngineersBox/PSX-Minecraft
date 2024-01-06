#include "world.h"

#include "../util/cvector_utils.h"

void __chunkDestructor(void *element) {
    chunkDestroy(element);
}

void worldInit(World *world) {
    world->chunks = NULL;
    cvector_init(world->chunks, 3, __chunkDestructor);
}

void worldRender(const World *world, DisplayContext *ctx, Transforms *transforms) {
    Chunk *chunk;
    cvector_for_each_in(chunk, world->chunks) {
        chunkRender(chunk, ctx, transforms);
    }
}

void worldLoadChunks(World* world, const VECTOR* player_pos) {

}

BlockID worldGetChunkBlock(const ChunkBlockPosition *position) {

}

BlockID worldGetBlock(const VECTOR *position) {

}
