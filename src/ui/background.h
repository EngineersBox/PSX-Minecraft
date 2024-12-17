#pragma once

#ifndef PSXMC__UI__BACKGROUND_H
#define PSXMC__UI__BACKGROUND_H

#include "../render/render_context.h"
#include "../util/inttypes.h"

void backgroundDraw(RenderContext* ctx, const u32* ot_object, const i8 u, const i8 v);

#endif // PSXMC__UI__BACKGROUND_H
