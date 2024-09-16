#include "subdivide.h"
#include "psxgpu.h"
#include "render_context.h"

typedef struct _DvisionLayout {
    u32 x_top;
    u32 x_bottom;
    u32 y_left;
    u32 y_right;
} DivisionLayout;

void subdividePolyFT4(SVECTOR* v0, SVECTOR* v1, SVECTOR* v2, SVECTOR* v3,
                      BDVECTOR* uv0, BDVECTOR* uv1, BDVECTOR* uv2, BDVECTOR* uv3,
                      DIVPOLY4* divp,
                      RenderContext* ctx,
                      const Transforms* transforms) {
    const DivisionLayout pos_div = (DivisionLayout) {
        (v1->vx - v0->vx) / divp->n_div_x,
        (v3->vx - v2->vx) / divp->n_div_x,
        (v2->vy - v0->vy) / divp->n_div_x,
        (v3->vy - v1->vy) / divp->n_div_x
    };
    const DivisionLayout tex_div = (DivisionLayout) {
        (uv1->u - uv0->u) / divp->n_div_x,
        (uv3->u - uv2->u) / divp->n_div_x,
        (uv2->v - uv0->v) / divp->n_div_x,
        (uv3->v - uv1->v) / divp->n_div_x
    };
    for (u32 i = 1; i <= divp->n_div_x; i++) {
        for (u32 j = 1; j <= divp->n_div_y; j++) {
            POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
            setPolyFT4(pol4);
            setXY4(
                pol4,
                v0->vx + (pos_div.x_top * (i - 1)),
                v0->vy + (pos_div.y_left * (i - 1)),
                v0->vx + (pos_div.x_top * i),
                v0->vy + (pos_div.y_left * (i - 1)),
                v0->vx + (pos_div.x_bottom * (i - 1)),
                v0->vy + (pos_div.y_right * i),
                v0->vx + (pos_div.x_bottom * i),
                v0->vy + (pos_div.y_right * i)
            );
            setRGB0(
                pol4,
                divp->rgbc.r,
                divp->rgbc.g,
                divp->rgbc.b
            );
            setUV4(
                pol4,
                uv0->u + (tex_div.x_top * (i - 1)),
                uv0->v + (tex_div.y_left * (i - 1)),
                uv0->u + (tex_div.x_top * i),
                uv0->v + (tex_div.y_left * (i - 1)),
                uv0->u + (tex_div.x_bottom * (i - 1)),
                uv0->v + (tex_div.y_right * i),
                uv0->u + (tex_div.x_bottom * i),
                uv0->v + (tex_div.y_right * i)
            );
            pol4->tpage = divp->tpage;
            pol4->clut = divp->clut;
            addPrim(divp->ot, pol4);
        }
    }
}
