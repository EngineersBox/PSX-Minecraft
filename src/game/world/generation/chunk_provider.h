#pragma once

#ifndef PSX_MINECRAFT_CHUNK_PROVIDER_H
#define PSX_MINECRAFT_CHUNK_PROVIDER_H

#include <interface99.h>
#include <stdbool.h>

#include "chunk_generator.h"

#define IChunkProvider_IFACE \
    vfunc(void, init, VSelf) \
    vfunc(void, destroy, VSelf) \
    vfunc(Chunk*, provide, VSelf, const VECTOR position) \
    vfunc(bool, save, VSelf, Chunk* chunk)

interface(IChunkProvider);

#define DEFN_CHUNK_PROVIDER(name, ...) \
    typedef struct { \
        IChunkGenerator generator; \
        __VA_ARGS__ \
    } name

#endif // PSX_MINECRAFT_CHUNK_PROVIDER_H
