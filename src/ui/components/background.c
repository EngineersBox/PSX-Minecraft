#include "background.h"

#include <psxgpu.h>

#include "../../structure/primitive/primitive.h"
#include "../../util/preprocessor.h"
#include "../../logging/logging.h"

UIBackground* uiBackgroundNew(const Texture* texture,
                              const DVECTOR position,
                              const DVECTOR dimensions,
                              const DVECTOR texture_coords,
                              const DVECTOR texture_dimensions,
                              const CVECTOR tint,
                              u8 ot_entry_index) {
    UIBackground* bg = malloc(sizeof(UIBackground));
    assert(bg != NULL);
    bg->component.position = position;
    bg->component.dimensions = dimensions;
    bg->texture = texture;
    bg->texture_coords = texture_coords;
    bg->texture_dimensions = texture_dimensions;
    bg->tint = tint;
    bg->ot_entry_index = ot_entry_index;
    return bg;
};

void uiBackgroundRender(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("UIBackground_render");
void UIBackground_render(VSelf, RenderContext* ctx, UNUSED Transforms* transforms) {
    VSELF(UIBackground);
    POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    setXYWH(
        pol4,
        self->component.position.vx,
        self->component.position.vy,
        self->component.dimensions.vx,
        self->component.dimensions.vy
    );
    setUVWH(
        pol4,
        self->texture_coords.vx,
        self->texture_coords.vy,
        self->component.dimensions.vx,
        self->component.dimensions.vy
    );
    // Mid point grey as mask for additive texturing
    setRGB0(
        pol4,
        self->tint.r,
        self->tint.g,
        self->tint.b
    );
    pol4->tpage = self->texture->tpage;
    pol4->clut = self->texture->clut;
    polyFT4Render(pol4, self->ot_entry_index, ctx);
    u8 mask_x = 0;
    switch (self->texture_dimensions.vx) {
        case 8: mask_x = TEXTURE_WINDOW_MASK_8; break;
        case 16: mask_x = TEXTURE_WINDOW_MASK_16; break;
        case 32: mask_x = TEXTURE_WINDOW_MASK_32; break;
        case 64: mask_x = TEXTURE_WINDOW_MASK_64; break;
        case 128: mask_x = TEXTURE_WINDOW_MASK_128; break;
        case 256: mask_x = TEXTURE_WINDOW_MASK_256; break;
        default:
            errorAbort("Unknown texture window mask size: %d\n", self->texture_dimensions.vx);
            break;
    }
    u8 mask_y = 0;
    switch (self->texture_dimensions.vx) {
        case 8: mask_y = TEXTURE_WINDOW_MASK_8; break;
        case 16: mask_y = TEXTURE_WINDOW_MASK_16; break;
        case 32: mask_y = TEXTURE_WINDOW_MASK_32; break;
        case 64: mask_y = TEXTURE_WINDOW_MASK_64; break;
        case 128: mask_y = TEXTURE_WINDOW_MASK_128; break;
        case 256: mask_y = TEXTURE_WINDOW_MASK_256; break;
        default:
            errorAbort("Unknown texture window mask size: %d\n", self->texture_dimensions.vx);
            break;
    }
    const TextureWindow tex_window = textureWindowCreateDirect(
        mask_x,
        mask_y,
        self->texture_coords.vx,
        self->texture_coords.vy
    );
    DR_TWIN* ptwin = (DR_TWIN*) allocatePrimitive(ctx, sizeof(DR_TWIN));
    setTexWindow(ptwin, &tex_window);
    u32* ot_object = allocateOrderingTable(ctx, self->ot_entry_index);
    addPrim(ot_object, ptwin);
    renderClearConstraintsIndex(ctx, self->ot_entry_index);
}
