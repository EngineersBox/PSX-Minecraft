#include "crosshair.h"

#include "../structure/primitive/primitive.h"

void crosshairDraw(RenderContext* ctx) {
    LINE_F2* vertical = (LINE_F2*) allocatePrimitive(ctx, sizeof(LINE_F2));
    setXY2(
        vertical,
        CENTRE_X, CENTRE_Y - 2,
        CENTRE_X, CENTRE_Y + 2
    );
    setRGB0(vertical, 0xff, 0xff, 0xff);
    LINE_F2* horizontal = (LINE_F2*) allocatePrimitive(ctx, sizeof(LINE_F2));
    setXY2(
        horizontal,
        CENTRE_X - 2, CENTRE_Y,
        CENTRE_X + 2, CENTRE_Y
    );
    setRGB0(horizontal, 0xff, 0xff, 0xff);
    lineF2Render(vertical, 0, ctx);
    lineF2Render(horizontal, 0, ctx);
}