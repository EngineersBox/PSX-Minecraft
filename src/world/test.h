#pragma once

#ifndef PSX_MINECRAFT_TEST_H
#define PSX_MINECRAFT_TEST_H

#include <stdio.h>
#include <inttypes.h>

#define RADIUS 2
#define CENTRE 1
#define SHIFT_ZONE 1
#define AXIS_SIZE (((RADIUS + SHIFT_ZONE) * 2) + CENTRE)

#define WORLD_HEIGHT_CHUNKS 3

#define arrayCoord(world, axis, value) ((RADIUS + SHIFT_ZONE + (world)->centre.axis) + value)

typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} VECTOR;

typedef struct {
    VECTOR centre;
    uint32_t chunks[AXIS_SIZE][AXIS_SIZE];
} World;

void printWorld(World* world) {
    for (int x = 0; x < AXIS_SIZE; x++) {
        for (int z = 0; z < AXIS_SIZE; z++) {
            printf(" %d ", world->chunks[x][z]);
        }
        printf("\n");
    }
}

int main() {
    // In chunks
    VECTOR player_pos = {0, 0, 0};
    World _world = {
        .centre = {
            .x = 0,
            .y = 0,
            .z = 0
        },
        .chunks = {0},
    };
    World* world = &_world;
    // Default loading of chunks
    for (int x = -RADIUS; x <= RADIUS; x++) {
        for (int z = -RADIUS; z <= RADIUS; z++) {
            world->chunks[arrayCoord(world, x, x)][arrayCoord(world, z, z)] = 1;
        }
    }
    printWorld(world);
    return 0;
}


#endif // PSX_MINECRAFT_TEST_H
