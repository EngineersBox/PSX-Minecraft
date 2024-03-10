#pragma once

#ifndef PSX_MINECRAFT_RENDERABLE_H
#define PSX_MINECRAFT_RENDERABLE_H

#include <interface99.h>

#include "render_context.h"
#include "transforms.h"

#define Renderable_IFACE \
    vfunc(void, renderWorld, VSelf, RenderContext* ctx, Transforms* transforms) \
    vfunc(void, renderInventory, VSelf, RenderContext* ctx, Transforms* transforms) \
    vfunc(void, renderHand, VSelf, RenderContext* ctx, Transforms* transforms)

interface(Renderable);

#endif // PSX_MINECRAFT_RENDERABLE_H
