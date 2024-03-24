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
    cvector_init(hotbar->slots, HOTBAR_SLOT_COUNT, NULL);
    for (uint8_t i = 0; i < 9; i++) {
        Slot* slot = &hotbar->slots[i];
        slot->dimensions = HOTBAR_SLOT_DIMS;
        slot->position = hotbarSlotPos(i, 0);
        slot->index = i;
    }
    cvector_push_back(hotbar->ui.components, (IUIComponent) {});
    IUIComponent* component = &hotbar->ui.components[cvector_size(hotbar->ui.components) - 1];
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
    DYN_PTR(component, UIBackground, IUIComponent, background);
}

void hotbarRenderSlots(const Hotbar* hotbar,  RenderContext* ctx, Transforms* transforms) {
    Slot* slot;
    cvector_for_each_in(slot, hotbar->slots) {
        if (slot->data.item == NULL) {
            continue;
        }
        Item* item = VCAST(Item*, *slot->data.item);
        item->position.vx = slot->position.vx;
        item->position.vy = slot->position.vy;
        VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
    }
    POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    setXYWH(
        pol4,
        CENTRE_X - (HOTBAR_WIDTH / 2) + (hotbar->selected_slot * (HOTBAR_SELECTOR_WIDTH - 2)),
        SCREEN_YRES - HOTBAR_HEIGHT - 1,
        HOTBAR_SELECTOR_WIDTH,
        HOTBAR_SELECTOR_HEIGHT
    );
    setUVWH(
        pol4,
        1,
        23,
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

void hotbarLoadTexture(VSelf) __attribute__((alias("Hotbar_loadTexture")));
void Hotbar_loadTexture(VSelf) {}

void hotbarFreeTexture(VSelf) __attribute__((alias("Hotbar_freeTexture")));
void Hotbar_freeTexture(VSelf) {}