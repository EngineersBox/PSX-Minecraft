#pragma once

#ifndef _PSXMC__WEATHER__WEATHER_H_
#define _PSXMC__WEATHER__WEATHER_H_

#include "../world/world_structure.h"
#include "../../entity/player.h"
#include "../../render/render_context.h"

// Number of blocks in each direction (excluding the centre)
// that weather is rendered in
#ifndef WEATHER_RENDER_RADIUS
#define WEATHER_RENDER_RADIUS 4
#endif

#define WEATHER_TEXTURE_WIDTH 64
#define WEATHER_TEXTURE_HALF_WIDTH (WEATHER_TEXTURE_WIDTH >> 1)
#define WEATHER_TEXTURE_HEIGHT 240

void weatherRender(const World* world,
                   const Player* player,
                   RenderContext* ctx,
                   Transforms* transforms);

#endif // _PSXMC__WEATHER__WEATHER_H_
