#include "tooltip.h"

#include "../../render/font.h"
#include "../../render/colours.h"
#include "../../ui/components/cursor.h"

void toolTipRender(RenderContext* ctx,
                   const char* text) {
    if (cursor.held_data != NULL || text == NULL) {
        return;
    }
    ctx->primitive = fontSort(
        allocateOrderingTable(ctx, 0),
        ctx->primitive,
        cursor.component.position.vx + (CURSOR_SPRITE_WIDTH >> 1) + 3,
        cursor.component.position.vy - 2,
        false,
        &COLOUR_BLACK,
        &COLOUR_BLACK,
        2,
        text
    );
}
