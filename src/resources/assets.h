#pragma once

#ifndef PSX_MINECRAFT_ASSETS_H
#define PSX_MINECRAFT_ASSETS_H

#include <psxgpu.h>

extern unsigned char _lz_resources[];
#define lz_resources ((const LZP_HEAD*) _lz_resources)

void assetsLoad();
void assetLoadImage(const TIM_IMAGE* tim);

#endif // PSX_MINECRAFT_ASSETS_H
