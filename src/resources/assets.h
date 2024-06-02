#pragma once

#ifndef PSX_MINECRAFT_ASSETS_H
#define PSX_MINECRAFT_ASSETS_H

#include <stdbool.h>
#include <psxgpu.h>
#include <smd/smd.h>

#include "texture.h"

extern uint8_t _lz_resources[];
#define lz_resources ((const LZP_HEAD*) _lz_resources)

extern Texture* textures;
extern bool assets_loaded;

void assetLoadImage(const TIM_IMAGE* tim, Texture* texture);
// 1: Failed, 0: Success
int assetLoadTextureDirect(const char* bundle, const char* filename, Texture* texture);
void assetLoadModel(const SMD* smd);

void assetsLoad();
void assetsFree();

#endif // PSX_MINECRAFT_ASSETS_H
