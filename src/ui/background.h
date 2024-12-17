#pragma once

#ifndef PSXMC__UI__BACKGROUND_H
#define PSXMC__UI__BACKGROUND_H

#include "../render/render_context.h"
#include "../util/inttypes.h"

void backgroundDraw(RenderContext* ctx, const u32* ot_object, const u8 u, const u8 v);

#endif // PSXMC__UI__BACKGROUND_H
