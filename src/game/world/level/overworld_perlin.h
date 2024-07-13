#pragma once

#ifndef _PSXMC__GAME_WORLD_LEVEL__OVERWORLD_PERLIN_H_
#define _PSXMC__GAME_WORLD_LEVEL__OVERWORLD_PERLIN_H_

#include <interface99.h>

#include "../generation/chunk_generator.h"
#include "../generation/chunk_provider.h"

DEFN_CHUNK_GENERATOR(OverworldPerlinChunkGenerator);

void overworldPerlinChunkGeneratorInit(VSelf);
void OverworldPerlinChunkGenerator_init(VSelf);

void overworldPerlinChunkGeneratorDestroy(VSelf);
void OverworldPerlinChunkGenerator_destroy(VSelf);

void overworldPerlinGeneneratorGenerate(VSelf, Chunk* chunk);
void OverworldPerlinChunkGenerator_generate(VSelf, Chunk* chunk);

impl(IChunkGenerator, OverworldPerlinChunkGenerator);

DEFN_CHUNK_PROVIDER(OverworldPerlinChunkProvider);

void overworldPerlinChunkProviderInit(VSelf);
void OverworldPerlinChunkProvider_init(VSelf);

void overworldPerlinChunkProviderDestroy(VSelf);
void OverworldPerlinChunkProvider_destroy(VSelf);

Chunk* overworldPerlinProvideChunk(VSelf, const VECTOR position);
Chunk* OverworldPerlinChunkProvider_provide(VSelf, const VECTOR position);

bool overworldPerlinSaveChunk(VSelf, Chunk* chunk);
bool OverworldPerlinChunkProvider_save(VSelf, Chunk* chunk);

impl(IChunkProvider, OverworldPerlinChunkProvider);

#endif // _PSXMC__GAME_WORLD_LEVEL__OVERWORLD_PERLIN_H_