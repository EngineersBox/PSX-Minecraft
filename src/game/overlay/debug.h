#pragma once

#ifndef _GAME_OVERLAY__DEBUG_H_
#define _GAME_OVERLAY__DEBUG_H_

#include "../world/world_structure.h"
#include "../../core/app_logic.h"
#include "../../core/camera.h"

void debugOverlayDraw(const Stats* stats, const Camera* camera, const World* world);

#endif // _GAME_OVERLAY__DEBUG_H_
