#include "hotbar.h"

#include "../../structure/cvector_utils.h"
#include "../../util/interface99_extensions.h"
#include "../../util/debounce.h"
#include "../../ui/components/background.h"
#include "../../structure/primitive/primitive.h"
#include "psxgpu.h"
#include "slot.h"

void hotbarInit(Hotbar* hotbar) {
    uiInit(&hotbar->ui);
    hotbar->ui.active = true;
    hotbar->ui.title = "hotbar";
    hotbar->selected_slot = 0;
    for (u8 i = slotGroupIndexOffset(HOTBAR); i < HOTBAR_SLOT_COUNT; i++) {
        Slot* slot = &hotbar->slots[i];
        slot->index = i;
        slot->data.item = NULL;
    }
    IUIComponent* component = uiAddComponent(&hotbar->ui);
    UIBackground* background = (UIBackground*) malloc(sizeof(UIBackground));
    background->component.position = (DVECTOR) {
        .vx = CENTRE_X - (HOTBAR_WIDTH / 2),
        .vy = SCREEN_YRES - HOTBAR_HEIGHT - 1
    };
    background->component.dimensions = (DVECTOR) {
        .vx = HOTBAR_WIDTH,
        .vy = HOTBAR_HEIGHT
    };
    background->texture_coords = (DVECTOR) {
        .vx = (16 * 16),
        .vy = 0
    };
    background->texture_width = (DVECTOR) {
        .vx = HOTBAR_WIDTH,
        .vy = HOTBAR_HEIGHT
    };
    background->texture = textures[ASSET_TEXTURE__STATIC__GUI_PART];
    DYN_PTR(component, UIBackground, IUIComponent, background);
}

void hotbarRenderSlots(const Hotbar* hotbar,  RenderContext* ctx, Transforms* transforms) {
    for (u8 y = 0; y < slotGroupDim(HOTBAR, Y); y++) {
        for (u8 x = 0; x < slotGroupDim(HOTBAR, X); x++) {
            const u8 i = slotGroupIndexOffset(HOTBAR) + (slotGroupDim(HOTBAR, X) * y) + x;
            const Slot* slot = &hotbar->slots[i];
            if (slot->data.item == NULL) {
                continue;
            }
            Item* item = VCAST_PTR(Item*,slot->data.item);
            item->position.vx = slotGroupScreenPosition(HOTBAR, X, x);
            item->position.vy = slotGroupScreenPosition(HOTBAR, Y, y);
            VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
        }
    }
    POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    setXYWH(
        pol4,
        CENTRE_X - (HOTBAR_WIDTH / 2) + (hotbar->selected_slot * 20) - 1,
        SCREEN_YRES - HOTBAR_HEIGHT - 2,
        HOTBAR_SELECTOR_WIDTH,
        HOTBAR_SELECTOR_HEIGHT
    );
    setUVWH(
        pol4,
        HOTBAR_WIDTH,
        0,
        HOTBAR_SELECTOR_WIDTH,
        HOTBAR_SELECTOR_HEIGHT
    );
    // Mid point grey as mask for additive texturing
    setRGB0(pol4, 0x80, 0x80, 0x80);
    const Texture* texture = &textures[ASSET_TEXTURE__STATIC__GUI_PART];
    pol4->tpage = texture->tpage;
    pol4->clut = texture->clut;
    polyFT4Render(pol4, 0, ctx);
    renderClearConstraintsIndex(ctx, 0);
}

#define FRAMES_PER_FLASH 2
#define FLASH_FRAMES (FRAMES_PER_FLASH * 4)
static u8 health_flash_animation = 0;

void hotbarRenderAttributes(u8 health,
                            bool health_start_flash,
                            u8 armour,
                            u8 air,
                            bool in_water,
                            RenderContext* ctx,
                            Transforms* transforms) {
    if (health_start_flash) {
        health_flash_animation = FLASH_FRAMES;
    } else if (health_flash_animation > 0) {
        health_flash_animation = clamp(
            health_flash_animation - 1,
            0,
            FLASH_FRAMES
        );
    }
    const Texture* texture = &textures[ASSET_TEXTURE__STATIC__GUI_PART];
    u32* ot_object = allocateOrderingTable(ctx, 1);
    POLY_FT4* pol4;
    // Heart fillers
    if (health > 0) {
        const u8 half_health = health % 2;
        const u8 full_health = health - half_health;
        const u8 health_width = ((full_health >> 1) * HOTBAR_HEALTH_ICON_WIDTH)
            + (half_health * HOTBAR_HEALTH_ICON_HALF_WIDTH);
        POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
        setPolyFT4(pol4);
        setXYWH(
            pol4,
            HOTBAR_HEALTH_ORIGIN_POS_X,
            HOTBAR_HEALTH_ORIGIN_POS_Y,
            health_width,
            HOTBAR_HEALTH_ICON_HEIGHT
        );
        setUVWH(
            pol4,
            HOTBAR_HEALTH_FILL_U,
            HOTBAR_HEALTH_FILL_V,
            health_width,            
            HOTBAR_HEALTH_ICON_HEIGHT
        );
        setRGB0(pol4, 0xFF, 0xFF, 0xFF);
        pol4->tpage = texture->tpage;
        pol4->clut = texture->clut;
        addPrim(ot_object, pol4);
    }
    // Heart borders
    pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    setPolyFT4(pol4);
    setXYWH(
        pol4,
        HOTBAR_HEALTH_ORIGIN_POS_X,
        HOTBAR_HEALTH_ORIGIN_POS_Y,
        HOTBAR_HEALTH_ICON_COUNT * HOTBAR_HEALTH_ICON_WIDTH,
        HOTBAR_HEALTH_ICON_HEIGHT
    );
    u16 health_u;
    u16 health_v;
    switch (health_flash_animation) {
        case FLASH_FRAMES - 1 ... FLASH_FRAMES:
            health_u = HOTBAR_HEALTH_WHITE_U;
            health_v = HOTBAR_HEALTH_WHITE_V;
            break;
        case FLASH_FRAMES - 3 ... FLASH_FRAMES - 2:
            health_u = HOTBAR_HEALTH_BLACK_U;
            health_v = HOTBAR_HEALTH_BLACK_V;
            break;
        case FLASH_FRAMES - 5 ... FLASH_FRAMES - 4:
            health_u = HOTBAR_HEALTH_WHITE_U;
            health_v = HOTBAR_HEALTH_WHITE_V;
            break;
        case FLASH_FRAMES - 7 ... FLASH_FRAMES - 6:
            health_u = HOTBAR_HEALTH_WHITE_U;
            health_v = HOTBAR_HEALTH_WHITE_V;
            break;
        default:
            health_u = HOTBAR_HEALTH_BLACK_U;
            health_v = HOTBAR_HEALTH_BLACK_V;
            break;
    }
    setUVWH(
        pol4,
        health_u,
        health_v,
        HOTBAR_HEALTH_ICON_COUNT * HOTBAR_HEALTH_ICON_WIDTH,
        HOTBAR_HEALTH_ICON_HEIGHT
    );
    setRGB0(pol4, 0xFF, 0xFF, 0xFF);
    pol4->tpage = texture->tpage;
    pol4->clut = texture->clut;
    addPrim(ot_object, pol4);
    if (armour > 0) {
        const u8 half_armour = armour % 2;
        const u8 full_armour = armour - half_armour;
        // Armour fillers
        pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
        setPolyFT4(pol4);
        const u8 armour_icons_width = ((full_armour >> 1) * HOTBAR_ARMOUR_ICON_WIDTH)
            + (half_armour * HOTBAR_ARMOUR_ICON_HALF_WIDTH);
        const u8 armour_icons_width_offset = (HOTBAR_ARMOUR_ICON_COUNT * HOTBAR_ARMOUR_ICON_WIDTH) - armour_icons_width;
        setXYWH(
            pol4,
            HOTBAR_ARMOUR_ORIGIN_POS_X + armour_icons_width_offset,
            HOTBAR_ARMOUR_ORIGIN_POS_Y,
            armour_icons_width,
            HOTBAR_ARMOUR_ICON_HEIGHT
        );
        setUVWH(
            pol4,
            HOTBAR_ARMOUR_FILL_U + armour_icons_width_offset,
            HOTBAR_ARMOUR_FILL_V,
            armour_icons_width,
            HOTBAR_ARMOUR_ICON_HEIGHT
        );
        setRGB0(pol4, 0xFF, 0xFF, 0xFF);
        pol4->tpage = texture->tpage;
        pol4->clut = texture->clut;
        addPrim(ot_object, pol4);
        // Armour borders
        pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
        setPolyFT4(pol4);
        setXYWH(
            pol4,
            HOTBAR_ARMOUR_ORIGIN_POS_X,
            HOTBAR_ARMOUR_ORIGIN_POS_Y,
            HOTBAR_ARMOUR_ICON_COUNT * HOTBAR_ARMOUR_ICON_WIDTH,
            HOTBAR_ARMOUR_ICON_HEIGHT
        );
        setUVWH(
            pol4,
            HOTBAR_ARMOUR_U,
            HOTBAR_ARMOUR_V,
            HOTBAR_ARMOUR_ICON_COUNT * HOTBAR_ARMOUR_ICON_WIDTH,
            HOTBAR_ARMOUR_ICON_HEIGHT
        );
        setRGB0(pol4, 0xFF, 0xFF, 0xFF);
        pol4->tpage = texture->tpage;
        pol4->clut = texture->clut;
        addPrim(ot_object, pol4);
    }
    if (!in_water || air <= 0) {
        return;
    }
    const bool half_air = air % 2;
    const u8 full_air = air - half_air;
    if (half_air) {
        // Air bubble pop
        pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
        setPolyFT4(pol4);
        setXYWH(
            pol4,
            HOTBAR_AIR_ORIGIN_POS_X + ((full_air >> 1) * HOTBAR_AIR_ICON_WIDTH),
            HOTBAR_AIR_ORIGIN_POS_Y,
            HOTBAR_AIR_ICON_WIDTH,
            HOTBAR_AIR_ICON_HEIGHT
        );
        setUVWH(
            pol4,
            HOTBAR_AIR_POP_U,
            HOTBAR_AIR_POP_V,
            HOTBAR_AIR_ICON_WIDTH,
            HOTBAR_AIR_ICON_HEIGHT
        );
        setRGB0(pol4, 0xFF, 0xFF, 0xFF);
        pol4->tpage = texture->tpage;
        pol4->clut = texture->clut;
        addPrim(ot_object, pol4);
    }
    // Air bubbles
    pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    setPolyFT4(pol4);
    setXYWH(
        pol4,
        HOTBAR_AIR_ORIGIN_POS_X,
        HOTBAR_AIR_ORIGIN_POS_Y,
        (full_air >> 1) * HOTBAR_AIR_ICON_WIDTH,
        HOTBAR_AIR_ICON_HEIGHT
    );
    setUVWH(
        pol4,
        HOTBAR_AIR_U,
        HOTBAR_AIR_V,
        HOTBAR_AIR_ICON_COUNT * HOTBAR_AIR_ICON_WIDTH,
        HOTBAR_AIR_ICON_HEIGHT
    );
    setRGB0(pol4, 0xFF, 0xFF, 0xFF);
    pol4->tpage = texture->tpage;
    pol4->clut = texture->clut;
    addPrim(ot_object, pol4);
}

void hotbarOpen(VSelf) ALIAS("Hotbar_open");
void Hotbar_open(VSelf) {
    // Always open
}

void hotbarClose(VSelf) ALIAS("Hotbar_close");
void Hotbar_close(VSelf) {
    // Always open
}

InputHandlerState hotbarInputHandler(const Input* input, void* ctx) {
    const PADTYPE* pad = input->pad;
    if (pad->stat != 0) {
        return INPUT_HANDLER_RELINQUISH_NO_DEBOUNCE;
    }
    Hotbar* hotbar = (Hotbar*) ctx;
    if (isPressed(pad, BINDING_PREVIOUS)
        && debounce(&hotbar->debounce, HOTBAR_DEBOUNCE_MS)) {
        hotbar->selected_slot = positiveModulo(((i8) hotbar->selected_slot) - 1, 9);
    } else if (isPressed(pad, BINDING_NEXT)
        && debounce(&hotbar->debounce, HOTBAR_DEBOUNCE_MS)) {
        hotbar->selected_slot = positiveModulo(((i8) hotbar->selected_slot) + 1, 9);
    }
    return INPUT_HANDLER_RELINQUISH_NO_DEBOUNCE;
}

void hotbarRegisterInputHandler(VSelf, Input* input, void* ctx) ALIAS("Hotbar_registerInputHandler");
void Hotbar_registerInputHandler(VSelf, Input* input, void* ctx) {
    VSELF(Hotbar);
    const InputHandlerVTable handler = (InputHandlerVTable) {
        .ctx = self,
        .input_handler = hotbarInputHandler
    };
    inputAddHandler(
        input,
        handler
    );
}
