#pragma once

#ifndef _PSXMC__RENDER__SUBDIVIDE_H_
#define _PSXMC__RENDER__SUBDIVIDE_H_

#include <psxgte.h>

#include "render_context.h"
#include "transforms.h"
#include "../math/vector.h"

typedef struct _DIVPOLY4 {
    u16 n_div_x; // X-coord subdivision count
    u16 n_div_y; // Y-coord subdivision count
    u16 clut;
    u16 tpage;
    CVECTOR rgbc;
    u32* ot;
} DIVPOLY4;

void subdividePolyFT4(SVECTOR* v0, SVECTOR* v1, SVECTOR* v2, SVECTOR* v3,
                      BDVECTOR* uv0, BDVECTOR* uv1, BDVECTOR* uv2, BDVECTOR* uv3,
                      DIVPOLY4* divp,
                      RenderContext* ctx,
                      const Transforms* transforms);

#endif // _PSXMC__RENDER__SUBDIVIDE_H_
