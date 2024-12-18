#include "items.h"

#include "item.h"
#include "item_id.h"

ItemConstructor item_constructors[ITEM_COUNT] = {0};
ItemAttributes item_attributes[ITEM_COUNT] = {0};

#define initItem(id, constructor, attributes) ({ \
    item_constructors[(id)] = constructor; \
    item_attributes[(id)] = attributes; \
})

void itemsInitialiseBuiltin() {
    initItem(
        ITEMID_STONE,
        itemConstructor(stone),
        stoneItemBlockAttributes()
    );
    initItem(
        ITEMID_DIRT,
        itemConstructor(dirt),
        dirtItemBlockAttributes()
    );
    initItem(
        ITEMID_GRASS,
        itemConstructor(grass),
        grassItemBlockAttributes()
    );
    initItem(
        ITEMID_COBBLESTONE,
        itemConstructor(cobblestone),
        cobblestoneItemBlockAttributes()
    );
    initItem(
        ITEMID_CRAFTING_TABLE,
        itemConstructor(craftingTable),
        craftingTableItemBlockAttributes()
    );
}
