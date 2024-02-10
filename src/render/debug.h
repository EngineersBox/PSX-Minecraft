#pragma once

#ifndef PSX_MINECRAFT_DEBUG_H
#define PSX_MINECRAFT_DEBUG_H

#include "../primitive/line.h"
#include "render_context.h"

void debugDrawOTUsageGraph(const RenderContext* ctx);
void debugDrawPBUsageGraph(const RenderContext* ctx);

#endif // PSX_MINECRAFT_DEBUG_H
