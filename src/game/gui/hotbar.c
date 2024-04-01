#include "hotbar.h"

#include "../../structure/cvector_utils.h"
#include "../../util/interface99_extensions.h"
#include "../../ui/components/background.h"
#include "../../structure/primitive/primitive.h"

void hotbarInit(Hotbar* hotbar) {
    uiInit(&hotbar->ui);
    hotbar->ui.active = true;
    hotbar->ui.title = "hotbar";
    hotbar->slots = NULL;
    hotbar->selected_slot = 1;
    cvector_init(hotbar->slots, 0, NULL);
    for (uint8_t i = 0; i < HOTBAR_SLOT_COUNT; i++) {
        cvector_push_back(hotbar->slots, (Slot){});
        Slot* slot = &hotbar->slots[i];
        slot->dimensions = HOTBAR_SLOT_DIMS;
        slot->position = hotbarSlotPos(i, 0);
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
    background->texture = &textures[ASSET_TEXTURES_GUI_INDEX];
    DYN_PTR(component, UIBackground, IUIComponent, background);
}

void hotbarRenderSlots(const Hotbar* hotbar,  RenderContext* ctx, Transforms* transforms) {
    Slot* slot;
    cvector_for_each_in(slot, hotbar->slots) {
        if (slot->data.item == NULL) {
            continue;
        }
        Item* item = VCAST_PTR(Item*, slot->data.item);
        item->position.vx = slot->position.vx;
        item->position.vy = slot->position.vy;
        VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
    }
    POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    setXYWH(
        pol4,
        CENTRE_X - (HOTBAR_WIDTH / 2) + (hotbar->selected_slot * 19),
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
    const Texture* texture = &textures[ASSET_TEXTURES_GUI_INDEX];
    pol4->tpage = texture->tpage;
    pol4->clut = texture->clut;
    polyFT4Render(pol4, 0, ctx);
    renderClearConstraintsIndex(ctx, 0);
}

void hotbarOpen(VSelf) __attribute__((alias("Hotbar_open")));
void Hotbar_open(VSelf) {
    // Always open
}

void hotbarClose(VSelf) __attribute__((alias("Hotbar_close")));
void Hotbar_close(VSelf) {
    // Always open
}

void hotbarRegisterHandler(VSelf, Input* input) __attribute__((alias("Hotbar_registerHandler")));
void Hotbar_registerHandler(VSelf, Input* input) {
    // Nothing to register
}
