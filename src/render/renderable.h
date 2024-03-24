#pragma once

#ifndef PSX_MINECRAFT_RENDERABLE_H
#define PSX_MINECRAFT_RENDERABLE_H

#include <interface99.h>

#include "render_context.h"
#include "transforms.h"

#define Renderable_IFACE \
    vfunc(void, renderWorld, VSelf, RenderContext* ctx, Transforms* transforms) \
    vfunc(void, applyWorldRenderAttributes, VSelf) \
    vfunc(void, renderInventory, VSelf, RenderContext* ctx, Transforms* transforms) \
    vfunc(void, applyInventoryRenderAttributes, VSelf) \
    vfunc(void, renderHand, VSelf, RenderContext* ctx, Transforms* transforms) \
    vfunc(void, applyHandRenderAttributes, VSelf)

interface(Renderable);

#endif // PSX_MINECRAFT_RENDERABLE_H
