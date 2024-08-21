#pragma once

#ifndef _PSXMC__WEATHER__WEATHER_H_
#define _PSXMC__WEATHER__WEATHER_H_

#include "../game/world/world_structure.h"
#include "../entity/player.h"
#include "../render/render_context.h"

// Number of blocks in each direction (excluding the centre)
// that weather is rendered in
#ifndef WEATHER_RENDER_RADIUS
#define WEATHER_RENDER_RADIUS 4
#endif

void renderWeatherOverlay(const World* world, const Player* player, RenderContext* ctx);

#endif // _PSXMC__WEATHER__WEATHER_H_
