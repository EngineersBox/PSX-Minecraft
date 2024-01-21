#pragma once

#ifndef PSX_MINECRAFT_ASSETS_H
#define PSX_MINECRAFT_ASSETS_H

#include <psxgpu.h>
#include <smd/smd.h>

#include "texture.h"

#define TERRAIN_TEXTURES 0

extern uint8_t _lz_resources[];
#define lz_resources ((const LZP_HEAD*) _lz_resources)

extern Texture* textures;

void assetsLoad();
void assetLoadImage(const TIM_IMAGE* tim, int index);
void assetLoadModel(const SMD* smd);
void assetsFree();

#endif // PSX_MINECRAFT_ASSETS_H
