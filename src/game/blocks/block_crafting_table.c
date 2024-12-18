#include "block_crafting_table.h"

#include "../../util/interface99_extensions.h"
#include "block.h"
#include "block_id.h"
#include "../items/blocks/item_block_crafting_table.h"
#include "../../logging/logging.h"

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
    self->block = declareSimpleBlock(BLOCKID_CRAFTING_TABLE);
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

static Slot crafting_table_sots[(slotGroupSize(CRAFTING_TABLE) + slotGroupSize(CRAFTING_TABLE_RESULT))] = {
    createSlotInline(CRAFTING_TABLE, 0, 0), createSlotInline(CRAFTING_TABLE, 1, 0), createSlotInline(CRAFTING_TABLE, 2, 0),
    createSlotInline(CRAFTING_TABLE, 0, 1), createSlotInline(CRAFTING_TABLE, 1, 0), createSlotInline(CRAFTING_TABLE, 2, 1),
    createSlotInline(CRAFTING_TABLE, 0, 2), createSlotInline(CRAFTING_TABLE, 1, 0), createSlotInline(CRAFTING_TABLE, 2, 2),
    createSlotInline(CRAFTING_TABLE_RESULT, 0, 0)
};

bool craftingTableBlockInputHandler(const Input* input, void* ctx) {
    const PADTYPE* pad = input->pad;
    if (isPressed(pad, BINDING_CURSOR_CLICK)) {
        // TODO: Use is either grabbing an item in a slot,
        //       putting a grabbed item in a slot, or
        //       mistargetting (with or without a grabbed
        //       item) with nothing happening. Handle this
        //       here, possibly with some logic that we share
        //       between this and the base inventory structure
        //       that is used for all inventories.
        return INPUT_HANDLER_RETAIN;
    } else if (isPressed(pad, BINDING_DROP_ITEM)) {
        // TODO: If can item is held, it should be dropped
        //       into the world.
    }
    if (isPressed(pad, BINDING_OPEN_INVENTORY)) {
        // Block inventory is closed, reset the render handlers
        // to stop rendering the overlay
        block_render_ui_context.function = NULL;
        block_render_ui_context.block = NULL;
        block_render_ui_context.background = (UIBackground) {0};
        return INPUT_HANDLER_RELIQUISH;
    }
    return INPUT_HANDLER_RETAIN;
}


bool craftingTableBlockUseAction(VSelf) ALIAS("CraftingTableBlock_useAction");
bool CraftingTableBlock_useAction(VSelf) {
    VSELF(IBlock);
    inputSetFocusedHandler(&input, &craftingTableBlockInputHandlerVTable);
    block_render_ui_context.function = craftingTableBlockRenderUI;
    block_render_ui_context.block = self;
    assetLoadTextureDirect(
        ASSET_BUNDLE__GUI,
        ASSET_TEXTURE__GUI__CRAFTING,
        &block_render_ui_context.background.texture
    );
    block_render_ui_context.background.texture_coords = vec2_i16_all(0);
    block_render_ui_context.background.texture_width = vec2_i16(
        CRAFTING_TABLE_TEXTURE_WIDTH,
        CRAFTING_TABLE_TEXTURE_HEIGHT
    );
    block_render_ui_context.background.component = (UIComponent) {
        .position = vec2_i16(
            CENTRE_X - (CRAFTING_TABLE_TEXTURE_WIDTH >> 1),
            CENTRE_Y - (CRAFTING_TABLE_TEXTURE_HEIGHT >> 1)
        ),
        .dimensions = vec2_i16(
            CRAFTING_TABLE_TEXTURE_WIDTH,
            CRAFTING_TABLE_TEXTURE_HEIGHT
        )
    };
    return false;
}

void craftingTableBlockRenderUI(RenderContext* ctx, Transforms* transforms) {
    // Render main storage slots and hotbar
    inventoryRenderSlots(
        VCAST_PTR(const Inventory*, block_input_handler_context.inventory),
        INVENTORY_SLOT_GROUP_MAIN | INVENTORY_SLOT_GROUP_HOTBAR,
        ctx,
        transforms
    );
    for (u8 y = 0; y < slotGroupDim(CRAFTING_TABLE, Y); y++) {
        for (u8 x = 0; x < slotGroupDim(CRAFTING_TABLE, X); x++) {
            const u8 i = slotGroupIndexOffset(CRAFTING_TABLE) + (slotGroupDim(CRAFTING_TABLE, X) * y) + x;
            const Slot* slot = &crafting_table_sots[i];
            if (slot->data.item == NULL) {
                continue;
            }
            Item* item = VCAST_PTR(Item*, slot->data.item);
            item->position.vx = slotGroupScreenPosition(CRAFTING_TABLE, X, x);
            item->position.vy = slotGroupScreenPosition(CRAFTING_TABLE, Y, y);
            VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
        }
    }
    const Slot* slot = &crafting_table_sots[slotGroupIndexOffset(CRAFTING_TABLE)];
    if (slot->data.item != NULL) {
        Item* item = VCAST_PTR(Item*, slot->data.item);
        item->position.vx = slotGroupScreenPosition(CRAFTING_TABLE, X, 0);
        item->position.vy = slotGroupScreenPosition(CRAFTING_TABLE, Y, 0);
        VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
    }
    uiBackgroundRender(
        &block_render_ui_context.background,
        ctx,
        transforms
    );
}
