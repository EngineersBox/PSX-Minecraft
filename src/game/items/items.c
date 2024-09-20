#include "items.h"

#include "blocks/item_block_dirt.h"
#include "blocks/item_block_grass.h"
#include "blocks/item_block_stone.h"
#include "item.h"
#include "item_id.h"

ItemConstructor item_constructors[ITEM_COUNT] = {NULL};
ItemAttributes item_attributes[ITEM_COUNT] = {0};

#define initItem(id, constructor, attributes) ({ \
    item_constructors[(id)] = constructor; \
    item_attributes[(id)] = attributes; \
})

void itemsInitialiseBuiltin() {
    initItem(
        ITEMID_STONE,
        stoneItemConstruct,
        stoneItemBlockAttributes()
    );
    initItem(
        ITEMID_DIRT,
        dirtItemConstruct,
        dirtItemBlockAttributes()
    );
    initItem(
        ITEMID_GRASS,
        grassItemConstruct,
        grassItemBlockAttributes()
    );
    initItem(
        ITEMID_COBBLESTONE,
        cobblestoneItemConstruct,
        cobblestoneItemBlockAttributes()
    );
}
