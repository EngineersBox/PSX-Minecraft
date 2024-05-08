#pragma once

#ifndef _PSX_MINECRAFT__GAME_WORLD_TYPE__OVERWORLD_FLATLAND_H_
#define _PSX_MINECRAFT__GAME_WORLD_TYPE__OVERWORLD_FLATLAND_H_

#include <interface99.h>

#include "../generation/chunk_generator.h"
#include "../generation/chunk_provider.h"

DEFN_CHUNK_GENERATOR(OverworldFlatlandChunkGenerator);

void overworldFlatlandChunkGeneratorInit(VSelf);
void OverworldFlatlandChunkGenerator_init(VSelf);

void overworldFlatlandChunkGeneratorDestroy(VSelf);
void OverworldFlatlandChunkGenerator_destroy(VSelf);

void overworldFlatlandGeneneratorGenerate(VSelf, Chunk* chunk);
void OverworldFlatlandChunkGenerator_generate(VSelf, Chunk* chunk);

impl(IChunkGenerator, OverworldFlatlandChunkGenerator);

DEFN_CHUNK_PROVIDER(OverworldFlatlandChunkProvider);

void overworldFlatlandChunkProviderInit(VSelf);
void OverworldFlatlandChunkProvider_init(VSelf);

void overworldFlatlandChunkProviderDestroy(VSelf);
void OverworldFlatlandChunkProvider_destroy(VSelf);

Chunk* overworldFlatlandProvideChunk(VSelf, const VECTOR position);
Chunk* OverworldFlatlandChunkProvider_provide(VSelf, const VECTOR position);

bool overworldFlatlandSaveChunk(VSelf, Chunk* chunk);
bool OverworldFlatlandChunkProvider_save(VSelf, Chunk* chunk);

impl(IChunkProvider, OverworldFlatlandChunkProvider);

#endif // _PSX_MINECRAFT__GAME_WORLD_TYPE__OVERWORLD_FLATLAND_H_