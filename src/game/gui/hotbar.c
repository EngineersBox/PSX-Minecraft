#include "hotbar.h"

#include "../../structure/cvector_utils.h"
#include "../../util/interface99_extensions.h"
#include "../../ui/components/background.h"
#include "../../structure/primitive/primitive.h"
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
    background->texture = textures[ASSET_TEXTURE__STATIC__GUI];
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
    const Texture* texture = &textures[ASSET_TEXTURE__STATIC__GUI];
    pol4->tpage = texture->tpage;
    pol4->clut = texture->clut;
    polyFT4Render(pol4, 0, ctx);
    renderClearConstraintsIndex(ctx, 0);
}

void hotbarOpen(VSelf) ALIAS("Hotbar_open");
void Hotbar_open(VSelf) {
    // Always open
}

void hotbarClose(VSelf) ALIAS("Hotbar_close");
void Hotbar_close(VSelf) {
    // Always open
}

static bool debounce(Hotbar* hotbar) {
    if (time_ms - hotbar->debounce >= HOTBAR_DEBOUNCE_MS) {
        hotbar->debounce = time_ms;
        return true;
    }
    return false;
}

bool hotbarInputHandler(const Input* input, void* ctx) {
    const PADTYPE* pad = input->pad;
    if (pad->stat != 0) {
        return false;
    }
    Hotbar* hotbar = (Hotbar*) ctx;
    if (isPressed(pad, BINDING_PREVIOUS) && debounce(hotbar)) {
        hotbar->selected_slot = positiveModulo(((i8) hotbar->selected_slot) - 1, 9);
    } else if (isPressed(pad, BINDING_NEXT) && debounce(hotbar)) {
        hotbar->selected_slot = positiveModulo(((i8) hotbar->selected_slot) + 1, 9);
    }
    return false;
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
