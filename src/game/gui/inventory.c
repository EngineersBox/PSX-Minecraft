#include "inventory.h"

void inventoryLoadTexture(VSelf) __attribute__((alias("Inventory_loadTexture")));
void Inventory_loadTexture(VSelf) {

}

void inventoryFreeTexture(VSelf) __attribute__((alias("Inventory_freeTexture")));
void Inventory_freeTexture(VSelf) {

}

void inventoryInit(Inventory* inventory, Hotbar* hotbar) {
    inventory->hotbar = hotbar;
    inventory->slots = NULL;
    cvector_init(inventory->slots, INVENTORY_SLOT_COUNT, NULL);
}