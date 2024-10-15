#pragma once

#ifndef PSXMC_MINECRAFT_H
#define PSXMC_MINECRAFT_H

#include <interface99.h>

#include "../core/app_logic.h"
#include "../core/camera.h"
#include "../core/input/input.h"
#include "../render/render_context.h"
#include "../render/transforms.h"
#include "world/world.h"

#ifndef PSXMC_VERSION_STRING
#define PSXMC_VERSION_STRING "PSXMC Infdev"
#endif

typedef struct {
    RenderContext ctx;
    Transforms transforms;
    Input input;
    Camera camera;
} Internals;

typedef struct {
    Internals internals;
    World* world;
} Minecraft;

void Minecraft_init(VSelf, void* ctx);
void minecraftInit(VSelf, void* ctx);

void Minecraft_cleanup(VSelf);
void minecraftCleanup(VSelf);

void Minecraft_input(VSelf, const Stats* stats);
void minecraftInput(VSelf, const Stats* stats);

void Minecraft_update(VSelf, const Stats* stats);
void minecraftUpdate(VSelf, const Stats* stats);

void Minecraft_render(VSelf, const Stats* stats);
void minecraftRender(VSelf, const Stats* stats);

impl(AppLogic, Minecraft);

#endif // PSXMC_MINECRAFT_H
