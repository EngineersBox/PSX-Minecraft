#pragma once

#ifndef PSXMC_CHUNK_GENERATOR_H
#define PSXMC_CHUNK_GENERATOR_H

#include <psxgte.h>
#include <interface99.h>

#include "../chunk/chunk.h"

#define IChunkGenerator_IFACE \
    vfunc(void, init, VSelf) \
    vfunc(void, destroy, VSelf) \
    vfunc(void, generate, VSelf, Chunk* chunk)

interface(IChunkGenerator);

#define DEFN_CHUNK_GENERATOR(name, ...) \
    typedef struct { \
        __VA_ARGS__ \
    } name

#endif // PSXMC_CHUNK_GENERATOR_H