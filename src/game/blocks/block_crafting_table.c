#include "block_crafting_table.h"

#include "../../util/interface99_extensions.h"
#include "block.h"
#include "block_id.h"
#include "../items/blocks/item_block_crafting_table.h"

bool craftingTableBlockInputHandler(const Input* input, void* ctx);
static InputHandlerVTable craftingTableBlockInputHandlerVTable = {
    .ctx = &block_input_handler_context,
    .input_handler = craftingTableBlockInputHandler,
    .input_handler_destroy = NULL
};

DEFN_BLOCK_CONSTRUCTOR_IMPL_STATELESS(craftingTable, CRAFTING_TABLE)

void craftingTableBlockInit(VSelf) ALIAS("CraftingTableBlock_init");
void CraftingTableBlock_init(VSelf) {
    VSELF(CraftingTableBlock);
    self->block = declareBlock(
        BLOCKID_CRAFTING_TABLE,
        0,
        0,
        opacityBitset(1,0,0,0,0,0),
        FACE_DIR_RIGHT
    );
}

IItem* craftingTableBlockDestroy(VSelf, bool drop_item) ALIAS("CraftingTableBlock_destroy");
IItem* CraftingTableBlock_destroy(VSelf, bool drop_item) {
    VSELF(CraftingTableBlock);
    return drop_item ? craftingTableBlockProvideItem(self) : NULL;
}

IItem* craftingTableBlockProvideItem(VSelf) ALIAS("CraftingTableBlock_provideItem");
IItem* CraftingTableBlock_provideItem(VSelf) {
    VSELF(CraftingTableBlock);
    IItem* item = itemCreate();
    CraftingTableItemBlock* item_block = craftingTableItemBlockCreate();
    DYN_PTR(item, CraftingTableItemBlock, IItem, item_block);
    VCALL(*item, init);
    itemBlockReplicateFaceAttributes(item_block->item_block, self->block);
    item_block->item_block.item.stack_size = 1;
    item_block->item_block.item.bob_direction = 1;
    return item;
}

bool craftingTableBlockInputHandler(const Input* input, void* ctx) {
    const PADTYPE* pad = input->pad;
    if (isPressed(pad, BINDING_USE)) {
        // TODO: Use is either grabbing an item in a slot,
        //       putting a grabbed item in a slot, or
        //       mistargetting (with or without a grabbed
        //       item) with nothing happening. Handle this
        //       here, possibly with some logic that we share
        //       between this and the base inventory structure
        //       that is used for all inventories.
        return false;
    }
    return !isPressed(pad, BINDING_OPEN_INVENTORY);
}


bool craftingTableBlockUseAction(VSelf) ALIAS("CraftingTableBlock_useAction");
bool CraftingTableBlock_useAction(VSelf) {
    inputSetFocusedHandler(&input, &craftingTableBlockInputHandlerVTable);
    // TODO: Create a global extern var that holds a function reference
    //       and an IBlock instance to be invoked to render a UI. This
    //       will be written to by a block when interacted with and set
    //       to NULL when empty (more specifically NULL both the function
    //       reference and IBlock instance). Then create a handler in
    //       this crafting table implementation to draw the background,
    //       then make a call to inventoryRenderSlots with just the MAIN
    //       and HOTBAR slot groups as well as rendering the actual 3x3
    //       crafting table input separately.
    return false;
}
