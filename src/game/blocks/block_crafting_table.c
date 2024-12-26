#include "block_crafting_table.h"

#include "../../util/interface99_extensions.h"
#include "block.h"
#include "block_id.h"
#include "../items/blocks/item_block_crafting_table.h"
#include "../../logging/logging.h"
#include "../../ui/components/cursor.h"
#include "../../entity/player.h"
#include "../items/items.h"
#include "../world/world_structure.h"

FWD_DECL Chunk* worldGetChunk(const World* world, const VECTOR* position);
FWD_DECL void worldDropItemStack(World* world, IItem* item, const u8 count);

Slot crafting_table_slots[(slotGroupSize(CRAFTING_TABLE) + slotGroupSize(CRAFTING_TABLE_RESULT))] = {0};

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

static void cursorInteractSlot(Slot* slot) {
    IItem* held_item = (IItem*) cursor.held_data;
    IItem* slot_item = slot->data.item;
    slot->data.item = held_item;
    cursor.held_data = slot_item;
}

static void cursorHandler(bool split_or_store_one) {
    if (!quadIntersectLiteral(
        &cursor.component.position,
        CENTRE_X - (CRAFTING_TABLE_TEXTURE_WIDTH >> 1),
        CENTRE_Y - (CRAFTING_TABLE_TEXTURE_HEIGHT >> 1),
        CRAFTING_TABLE_TEXTURE_WIDTH,
        CRAFTING_TABLE_TEXTURE_HEIGHT
    )) {
        // Outside window quad
        worldDropItemStack(
            world,
            cursor.held_data,
            0
        );
        return;
    }
    Slot* slot = NULL;
    if (slotGroupIntersect(CRAFTING_TABLE, &cursor.component.position)) {
        slot = &crafting_table_slots[slotGroupCursorSlot(
            CRAFTING_TABLE,
            &cursor.component.position
        )];
        if (split_or_store_one) {
            cursorSplitOrStoreOne(
                slot,
                slotDirectItemGetter,
                slotDirectItemSetter
            );
        } else {
            cursorInteractSlot(slot);
        }
    } else if (slotGroupIntersect(CRAFTING_TABLE_RESULT, &cursor.component.position)) {
        slot = &crafting_table_slots[slotGroupCursorSlot(
            CRAFTING_TABLE_RESULT,
            &cursor.component.position
        )];
        IItem* slot_iitem = slot->data.item;
        IItem* held_iitem = cursor.held_data;
        if (slot_iitem == NULL || held_iitem != NULL) {
            // WE should not be able to interact
            // with the result slot except for
            // pulling items out of it
            return;
        }
        Item* slot_item = VCAST_PTR(Item*, slot_iitem);
        if (!split_or_store_one || slot_item->stack_size == 1) {
            // If we are not splitting, or we have a single
            // element stack, then just move the  contents
            // into the cursor
            cursor.held_data = slot_iitem;
            return;
        }
        // Do the normal splitting of stacks between the
        // existing slot stack and a new stack
        IItem* split_stack = itemGetConstructor(slot_item->id)();
        assert(split_stack);
        Item* split_stack_item = VCAST_PTR(Item*, split_stack);
        split_stack_item->stack_size = slot_item->stack_size >> 1;
        itemSetWorldState(split_stack_item, false);
        cursor.held_data = split_stack;
        slot_item->stack_size -= split_stack_item->stack_size;
    }
}

bool craftingTableBlockInputHandler(const Input* input, void* ctx) {
    VCALL(cursor_component, update);
    inventoryCursorHandler(
        VCAST_PTR(Inventory*, block_input_handler_context.inventory),
        INVENTORY_SLOT_GROUP_MAIN | INVENTORY_SLOT_GROUP_HOTBAR,
        input
    );
    const PADTYPE* pad = input->pad;
    // TODO Determime the button layout on xbox/playstation/switch
    //      MC releases and match it here for interacting with items
    //      in inventories and dropping stuff
    if (isPressed(pad, BINDING_CURSOR_CLICK)) {
        // TODO: Use is either grabbing an item in a slot,
        //       putting a grabbed item in a slot, or
        //       mistargetting (with or without a grabbed
        //       item) with nothing happening. Handle this
        //       here, possibly with some logic that we share
        //       between this and the base inventory structure
        //       that is used for all inventories.
        //
        //       This should also handle exchanging a held item
        //       and an item in a targetted slot.
        // TODO: Item should be grabbed/put from/into a slot
        //       or do nothing if mistargetted
        cursorHandler(false);
        return INPUT_HANDLER_RETAIN;
    } else if (isPressed(pad, BINDING_DROP_ITEM) && cursor.held_data != NULL) {
        worldDropItemStack(
            world,
            (IItem*) cursor.held_data,
            0
        );
    } else if (isPressed(pad, BINDING_SPLIT_OR_STORE_ONE)) {
        cursorHandler(true);
    }
    if (isPressed(pad, BINDING_OPEN_INVENTORY)) {
        // Block inventory is closed, reset the render handlers
        // to stop rendering the overlay
        block_render_ui_context.function = NULL;
        block_render_ui_context.block = NULL;
        block_render_ui_context.background = (UIBackground) {0};
        return INPUT_HANDLER_RELINQUISH;
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
            const Slot* slot = &crafting_table_slots[i];
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
    uiCursorRender(
        &cursor,
        ctx,
        transforms
    );
}
