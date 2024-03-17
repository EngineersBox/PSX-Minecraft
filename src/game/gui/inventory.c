#include "inventory.h"

void inventoryInit(Inventory* inventory, Hotbar* hotbar) {
    inventory->hotbar = hotbar;
    inventory->slots = NULL;
    cvector_init(inventory->slots, INVENTORY_SLOT_COUNT, NULL);
}

void inventoryRender(const Inventory* inventory, RenderContext* ctx, Transforms* transforms) {
    uiRender(&inventory->ui, ctx, transforms);
}