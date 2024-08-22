#pragma once

#ifndef _PSXMC__GAME_WORLD_LEVEL__OVERWORLD_FLATLAND_H_
#define _PSXMC__GAME_WORLD_LEVEL__OVERWORLD_FLATLAND_H_

#include <interface99.h>

#include "../generation/chunk_generator.h"
#include "../generation/chunk_provider.h"

DEFN_CHUNK_GENERATOR(OverworldFlatlandChunkGenerator);

void overworldFlatlandChunkGeneratorInit(VSelf);
void OverworldFlatlandChunkGenerator_init(VSelf);

void overworldFlatlandChunkGeneratorDestroy(VSelf);
void OverworldFlatlandChunkGenerator_destroy(VSelf);

void overworldFlatlandGeneneratorGenerate(VSelf, Chunk* chunk, ChunkHeightmap* heightmap);
void OverworldFlatlandChunkGenerator_generate(VSelf, Chunk* chunk, ChunkHeightmap* heightmap);

impl(IChunkGenerator, OverworldFlatlandChunkGenerator);

DEFN_CHUNK_PROVIDER(OverworldFlatlandChunkProvider);

void overworldFlatlandChunkProviderInit(VSelf);
void OverworldFlatlandChunkProvider_init(VSelf);

void overworldFlatlandChunkProviderDestroy(VSelf);
void OverworldFlatlandChunkProvider_destroy(VSelf);

Chunk* overworldFlatlandProvideChunk(VSelf, const VECTOR position, ChunkHeightmap* heightmap);
Chunk* OverworldFlatlandChunkProvider_provide(VSelf, const VECTOR position, ChunkHeightmap* heightmap);

bool overworldFlatlandSaveChunk(VSelf, Chunk* chunk);
bool OverworldFlatlandChunkProvider_save(VSelf, Chunk* chunk);

impl(IChunkProvider, OverworldFlatlandChunkProvider);

#endif // _PSXMC__GAME_WORLD_LEVEL__OVERWORLD_FLATLAND_H_
