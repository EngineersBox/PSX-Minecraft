#include "block_crafting_table.h"

#include "../../util/interface99_extensions.h"
#include "block.h"
#include "block_id.h"
#include "../items/blocks/item_block_crafting_table.h"
#include "../../logging/logging.h"
#include "../../ui/components/cursor.h"
#include "../items/items.h"
#include "../world/world_structure.h"
#include "../recipe/crafting.h"

FWD_DECL Chunk* worldGetChunk(const World* world, const VECTOR* position);
FWD_DECL void worldDropItemStack(World* world, IItem* item, const u8 count);

Slot crafting_table_slots[(slotGroupSize(CRAFTING_TABLE) + slotGroupSize(CRAFTING_TABLE_RESULT))] = {
    createSlotInline(CRAFTING_TABLE, 0, 0), createSlotInline(CRAFTING_TABLE, 1, 0), createSlotInline(CRAFTING_TABLE, 2, 0),
    createSlotInline(CRAFTING_TABLE, 0, 1), createSlotInline(CRAFTING_TABLE, 1, 0), createSlotInline(CRAFTING_TABLE, 2, 1),
    createSlotInline(CRAFTING_TABLE, 0, 2), createSlotInline(CRAFTING_TABLE, 1, 0), createSlotInline(CRAFTING_TABLE, 2, 2),
    createSlotInline(CRAFTING_TABLE_RESULT, 0, 0)
};

InputHandlerState craftingTableBlockInputHandler(const Input* input, void* ctx);
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

static bool processCraftingRecipe() {
    // TODO: Only invoke this when something changes in the crafting grid
    //       or output slot
    RECIPE_PATTERN(pattern, 9) = {0};
    for (int i = 0; i < slotGroupIndexOffset(CRAFTING_TABLE_RESULT); i++) {
        const Slot* slot = &crafting_table_slots[i];
        const IItem* iitem = slot->data.item;
        if (iitem != NULL) {
            const Item* item = VCAST_PTR(Item*, iitem);
            pattern[i] = (RecipePatternEntry) {
                .separated.metadata = item->metadata_id,
                .separated.id = item->id
            };
        } else {
            pattern[i] = (RecipePatternEntry) {
                .separated.metadata = 0,
                .separated.id = ITEMID_AIR
            };
        }
    }
    Slot* output_slot = &crafting_table_slots[slotGroupIndexOffset(CRAFTING_TABLE_RESULT)];
    return recipeProcessGrid(
        crafting_recipes,
        pattern,
        (Dimension){ .width = 3, .height = 3 },
        &output_slot,
        1,
        false
    );
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
        uiCursorSetHeldData(&cursor, NULL);
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
            cursorInteractSlot(
                slot,
                slotDirectItemGetter,
                slotDirectItemSetter
            );
        }
    } else if (slotGroupIntersect(CRAFTING_TABLE_RESULT, &cursor.component.position) && !split_or_store_one) {
        // NOTE: Don't bother with splitting stacks
        //       since it's a pain for the output slot.
        Slot* result_slot = &crafting_table_slots[slotGroupIndexOffset(CRAFTING_TABLE_RESULT)];
        IItem* result_iitem = result_slot->data.item;
        if (result_iitem == NULL) {
            return;
        }
        Item* result_item = VCAST_PTR(Item*, result_iitem);
        IItem* held_iitem = (IItem*) cursor.held_data;
        if (held_iitem == NULL) {
            recipeConsumeIngredients(
                crafting_table_slots,
                slotGroupIndexOffset(CRAFTING_TABLE),
                slotGroupIndexOffset(CRAFTING_TABLE_RESULT)
            );
            uiCursorSetHeldData(&cursor, result_iitem);
            result_slot->data.item = NULL;
            return;
        } 
        Item* held_item = VCAST_PTR(Item*, held_iitem);
        if (itemEquals(held_item, result_item)) {
            // Held and result item ids mismatch
            return;
        }
        held_item->stack_size += result_item->stack_size;
        VCALL(*result_iitem, destroy);
        result_slot->data.item = NULL;
        recipeConsumeIngredients(
            crafting_table_slots,
            slotGroupIndexOffset(CRAFTING_TABLE),
            slotGroupIndexOffset(CRAFTING_TABLE_RESULT)
        );
    }
}

InputHandlerState craftingTableBlockInputHandler(const Input* input, void* ctx) {
    processCraftingRecipe();
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
        cursorHandler(false);
        return INPUT_HANDLER_RETAIN;
    } else if (isPressed(pad, BINDING_DROP_ITEM) && cursor.held_data != NULL) {
        worldDropItemStack(
            world,
            (IItem*) cursor.held_data,
            0
        );
        uiCursorSetHeldData(&cursor, NULL);
    } else if (isPressed(pad, BINDING_SPLIT_OR_STORE_ONE)) {
        cursorHandler(true);
    }
    if (isPressed(pad, BINDING_OPEN_INVENTORY)) {
        // Block inventory is closed, reset the render handlers
        // to stop rendering the overlay
        block_render_ui_context.function = NULL;
        block_render_ui_context.block = NULL;
        block_render_ui_context.background = (UIBackground) {0};
        // Drop all ingredients into the world and
        // destroy the output slot item
        for (int i = slotGroupIndexOffset(CRAFTING_TABLE);
            i < slotGroupIndexOffset(CRAFTING_TABLE_RESULT); i++) {
            Slot* slot = &crafting_table_slots[i];
            if (slot->data.item == NULL) {
                continue;
            }
            worldDropItemStack(
                world,
                (IItem*)  slot->data.item,
                0
            );
            slot->data.item = NULL;
        }
        Slot* output_slot = &crafting_table_slots[slotGroupIndexOffset(CRAFTING_TABLE_RESULT)];
        if (output_slot->data.item != NULL) {
            VCALL(*((IItem*) output_slot->data.item), destroy);
            output_slot->data.item = NULL;
        }
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
    return BLOCK_USE_ACTION_CONSUMED;
}

void craftingTableBlockRenderUI(RenderContext* ctx, Transforms* transforms) {
    // Render main storage slots and hotbar
    uiCursorRender(
        &cursor,
        ctx,
        transforms
    );
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
    const Slot* slot = &crafting_table_slots[slotGroupIndexOffset(CRAFTING_TABLE_RESULT)];
    if (slot->data.item != NULL) {
        Item* item = VCAST_PTR(Item*, slot->data.item);
        item->position.vx = slotGroupScreenPosition(CRAFTING_TABLE_RESULT, X, 0);
        item->position.vy = slotGroupScreenPosition(CRAFTING_TABLE_RESULT, Y, 0);
        VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
    }
    uiBackgroundRender(
        &block_render_ui_context.background,
        ctx,
        transforms
    );
}
