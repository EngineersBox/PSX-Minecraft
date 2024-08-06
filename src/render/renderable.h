#pragma once

#ifndef PSXMC_RENDERABLE_H
#define PSXMC_RENDERABLE_H

#include <interface99.h>

#include "render_context.h"
#include "transforms.h"

FWD_DECL typedef struct Chunk Chunk;

#define Renderable_IFACE \
    vfunc(void, renderWorld, VSelf, const Chunk* chunk, RenderContext* ctx, Transforms* transforms) \
    vfunc(void, applyWorldRenderAttributes, VSelf) \
    vfunc(void, renderInventory, VSelf, RenderContext* ctx, Transforms* transforms) \
    vfunc(void, applyInventoryRenderAttributes, VSelf) \
    vfunc(void, renderHand, VSelf, RenderContext* ctx, Transforms* transforms) \
    vfunc(void, applyHandRenderAttributes, VSelf)

interface(Renderable);

#endif // PSXMC_RENDERABLE_H
