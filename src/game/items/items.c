#include "items.h"

#include "item_id.h"

ItemAttributes item_attributes[ITEM_COUNT] = {0};

#define initItemAttributes(id, attributes) ({ \
    item_attributes[id] = attributes; \
})

void itemsInitialiseBuiltin() {
    initItemAttributes(
        ITEMID_STONE,
        stoneItemBlockAttributes()
    );
    initItemAttributes(
        ITEMID_DIRT,
        dirtItemBlockAttributes()
    );
    initItemAttributes(
        ITEMID_GRASS,
        grassItemBlockAttributes()
    );
    initItemAttributes(
        ITEMID_COBBLESTONE,
        cobblestoneItemBlockAttributes()
    );
}