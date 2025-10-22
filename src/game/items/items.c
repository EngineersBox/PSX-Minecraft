#include "items.h"

#include "item.h"
#include "item_id.h"

// TODO: I think both arrays should be constant and initialised
//       at compile time. We know all of the ctor's ahead of time
//       so there is no point in leaving it to runtime init with
//       the helper macros. A bit more dev pain, but worth the
//       cost to allow the compiler to inline all usages of array 
//       elements at compile time.
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
        ITEMID_PLANK,
        itemConstructor(plank),
        plankItemBlockAttributes()
    );
    initItem(
        ITEMID_CRAFTING_TABLE,
        itemConstructor(craftingTable),
        craftingTableItemBlockAttributes()
    );
}
