#pragma once

#ifndef PSX_MINECRAFT_LINE_H
#define PSX_MINECRAFT_LINE_H

#include <psxgpu.h>

#include "../render/render_context.h"

void lineG2Render(const LINE_G2* line, int ot_entry, RenderContext* ctx);

#endif // PSX_MINECRAFT_LINE_H
