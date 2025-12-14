#include "progress_bar.h"

#include "../math/fixed_point.h"
#include "../structure/primitive/primitive.h"

void progressBarRender(const ProgressBar* bar, const int ot_entry, RenderContext* ctx) {
    // NOTE: Progress bar created in reverse order in ordering table
    //       since OT works in reverse
    // Progress bar marker
    i32 progress = (bar->dimensions.width << FIXED_POINT_SHIFT) / bar->maximum;
    progress *= bar->value;
    progress >>= FIXED_POINT_SHIFT;
    POLY_F4* poly_f4 = (POLY_F4*) allocatePrimitive(ctx, sizeof(POLY_F4));
    setXY4(
        poly_f4,
        bar->position.x, bar->position.y,
        bar->position.x + progress, bar->position.y,
        bar->position.x , bar->position.y + bar->dimensions.height,
        bar->position.x + progress, bar->position.y + bar->dimensions.height
    );
    setRGB0(poly_f4, 0x0, 0xff, 0x0);
    polyF4Render(poly_f4, ot_entry, ctx);
    // Bar background
    poly_f4 = (POLY_F4*) allocatePrimitive(ctx, sizeof(POLY_F4));
    setXY4(
        poly_f4,
        bar->position.x, bar->position.y,
        bar->position.x + bar->dimensions.width, bar->position.y,
        bar->position.x, bar->position.y + bar->dimensions.height,
        bar->position.x + bar->dimensions.width, bar->position.y + bar->dimensions.height
    );
    setRGB0(poly_f4, 0x77, 0x77, 0x77);
    polyF4Render(poly_f4, ot_entry, ctx);
    // // Outline
    // poly_f4 = (POLY_F4*) allocatePrimitive(ctx, sizeof(POLY_F4));
    // setXY4(
    //     poly_f4,
    //     bar->position.x - 1, bar->position.y - 1,
    //     bar->position.x + bar->dimensions.width + 1, bar->position.y - 1,
    //     bar->position.x - 1, bar->position.y + bar->dimensions.height + 1,
    //     bar->position.x + bar->dimensions.width + 1, bar->position.y + bar->dimensions.height + 1
    // );
    // setRGB0(poly_f4, 0xff, 0xff, 0xff);
    // polyF4Render(poly_f4, ot_entry, ctx);
}
