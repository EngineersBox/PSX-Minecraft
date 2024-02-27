#pragma once

#ifndef PSX_MINECRAFT_APP_LOGIC_H
#define PSX_MINECRAFT_APP_LOGIC_H

#include <interface99.h>
#include <stdbool.h>

#include "../render/render_context.h"
#include "../render/transforms.h"
#include "camera.h"

#define AppLogic_IFACE \
    vfunc(void, cleanup, VSelf) \
    vfunc(void, init, VSelf, void* ctx) \
    vfunc(void, input, VSelf, RenderContext* ctx, Transforms* transforms, Camera* camera, uint16_t diffTime, bool inputConsumed) \
    vfunc(void, update, VSelf, RenderContext* ctx, Transforms* transforms, Camera* camera, uint16_t diffTime)

interface(AppLogic);

#endif // PSX_MINECRAFT_APP_LOGIC_H
