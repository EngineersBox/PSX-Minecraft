#include "block_furnace.h"

#include "block.h"
#include "block_id.h"
#include "../items/blocks/item_block_furnace.h"
#include "../items/items.h"
#include "../gui/inventory.h"
#include "../gui/tooltip.h"
#include "../gui/utils.h"
#include "../recipe/furnace.h"
#include "../world/world_structure.h"
#include "../../core/input/input.h"
#include "../../logging/logging.h"
#include "../../ui/components/cursor.h"
#include "../../util/interface99_extensions.h"

static Texture furnace_texture = {0};

Slot furnace_slots[
    slotGroupSize(FURNACE_INPUT)
    + slotGroupSize(FURNACE_FUEL)
    + slotGroupSize(FURNACE_OUTPUT)
] = {
    createSlotInline(FURNACE_INPUT, 0, 0),
    createSlotInline(FURNACE_FUEL, 0, 0),
    createSlotInline(FURNACE_OUTPUT, 0, 0)
};

FWD_DECL Chunk* worldGetChunk(const World* world, const VECTOR* position);
FWD_DECL void worldDropItemStack(World* world, IItem* item, const u8 count);

InputHandlerState furnaceBlockInputHandler(const Input* input, void* ctx);
static InputHandlerVTable furnaceBlockInputHandlerVTable = {
    .ctx = &block_input_handler_context,
    .input_handler = furnaceBlockInputHandler,
    .input_handler_destroy = NULL
};

static u8 ingredient_consume_sizes[slotGroupSize(FURNACE_INPUT) + slotGroupSize(FURNACE_FUEL)] = {0};
static RECIPE_PATTERN(pattern, slotGroupSize(FURNACE_INPUT) + slotGroupSize(FURNACE_FUEL)) = {0};

DEFN_BLOCK_CONSTRUCTOR_IMPL_STATEFUL(furnace) {
    if (from_item != NULL) {
        Item* item = VCAST_PTR(Item*, from_item);
        item->stack_size--;
    }
    IBlock* iblock = iblockCreate();
    FurnaceBlock* furnace_block = malloc(sizeof(FurnaceBlock));
    DYN_PTR(iblock, FurnaceBlock, IBlock, furnace_block);
    VCALL(*iblock, init);
    return iblock;
}

void furnaceBlockInit(VSelf) ALIAS("FurnaceBlock_init");
void FurnaceBlock_init(VSelf) {
    VSELF(FurnaceBlock);
    self->block = declareBlock(
        BLOCKID_FURNACE,
        .light_level = 0,
        .orientation = FACE_DIR_FRONT,
        ._pad = 0
    );
    self->cook_ticks = 0;
    self->fuel_burn_ticks = 0;
    self->recipe = (RecipeQueryResult) {
        .result_count = 0,
        .results = NULL
    };
}

IItem* furnaceBlockDestroy(VSelf, bool drop_item) ALIAS("FurnaceBlock_destroy");
IItem* FurnaceBlock_destroy(VSelf, bool drop_item) {
    VSELF(FurnaceBlock);
    return drop_item ? furnaceBlockProvideItem(self) : NULL;
}

IItem* furnaceBlockProvideItem(VSelf) ALIAS("FurnaceBlock_provideItem");
IItem* FurnaceBlock_provideItem(VSelf) {
    VSELF(FurnaceBlock);
    IItem* item = itemConstructor(furnace)(0);
    FurnaceItemBlock* item_block = VCAST_PTR(FurnaceItemBlock*, item);
    itemBlockReplicateFaceAttributes(item_block->item_block, self->block);
    item_block->item_block.item.stack_size = 1;
    item_block->item_block.item.bob_direction = 1;
    return item;
}

void furnaceBlockUpdate(VSelf) ALIAS("FurnaceBlock_update");
void FurnaceBlock_update(VSelf) {
    // TODO: Handle fuel burn, cooking, recipe processing here.
    UNIMPLEMENTED();
}

static void processFurnaceRecipe(FurnaceBlock* furnace) {
    if (!furnace->recipe_changed) {
        return;
    }
    memset(ingredient_consume_sizes, '\0', sizeof(u8) * slotGroupSize(FURNACE_OUTPUT));
    for (int i = 0; i < slotGroupIndexOffset(FURNACE_OUTPUT); i++) {
        const Slot* slot = &furnace->furnace_slots[i];
        const IItem* iitem = slot->data.item;
        if (iitem != NULL) {
            const Item* item = VCAST_PTR(Item*, iitem);
            pattern[i] = (RecipePatternEntry) {
                .id = RECIPE_COMPOSITE_ID(item->id, item->metadata_id),
                .stack_size = item->stack_size,
            };
        } else {
            pattern[i] = (RecipePatternEntry) {
                .id = RECIPE_COMPOSITE_ID(0, ITEMID_AIR),
                .stack_size = 0,
            };
        }
    }
    const RecipeQueryState result = recipeSearch(
        furnace_recipes,
        pattern,
        (Dimension){
            .width = slotGroupDim(FURNACE_INPUT, X),
            .height = slotGroupDim(FURNACE_INPUT, Y)
        },
        &furnace->recipe,
        ingredient_consume_sizes,
        false
    );
    switch (result) {
        case RECIPE_FOUND:
            break;
        case RECIPE_NOT_FOUND:
            furnace->recipe.result_count = 0;
            furnace->recipe.results = NULL;
            break;
    }
    furnace->recipe_changed = false;
}

void cursorHandler(FurnaceBlock* furnace,
                   const bool split_or_store_one) {
    if (!quadIntersectLiteral(
        &cursor.component.position,
        CENTRE_X - (FURNACE_TEXTURE_WIDTH >> 1),
        CENTRE_Y - (FURNACE_TEXTURE_HEIGHT >> 1),
        FURNACE_TEXTURE_WIDTH,
        FURNACE_TEXTURE_HEIGHT
    )) {
        worldDropItemStack(
            world,
            (IItem*) cursor.held_data,
            0
        );
        uiCursorSetHeldData(&cursor, NULL);
        return;
    }
    Slot* slot = NULL;
    if (slotGroupIntersect(FURNACE_INPUT, &cursor.component.position)) {
        slot = &furnace->furnace_slots[slotGroupIndexOffset(FURNACE_INPUT)];
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
        furnace->recipe_changed = true;
        furnace->cook_ticks = 0;
    } else if (slotGroupIntersect(FURNACE_FUEL, &cursor.component.position)) {
        slot = &furnace->furnace_slots[slotGroupIndexOffset(FURNACE_FUEL)];
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
        furnace->recipe_changed = false;
    } else if (slotGroupIntersect(FURNACE_OUTPUT, &cursor.component.position) && !split_or_store_one) {
        // NOTE: Don't bother with splitting stacks
        //       since it's a pain the for output slot.
        slot = &furnace->furnace_slots[slotGroupIndexOffset(FURNACE_OUTPUT)];
        IItem* result_iitem = slot->data.item;
        if (result_iitem == NULL) {
            return;
        }
        Item* result_item = VCAST_PTR(Item*, result_iitem);
        IItem* held_iitem = (IItem*) cursor.held_data;
        if (held_iitem == NULL) {
            recipeConsumeIngredients(
                furnace->furnace_slots,
                ingredient_consume_sizes,
                slotGroupIndexOffset(FURNACE_INPUT),
                slotGroupIndexOffset(FURNACE_FUEL)
            );
            uiCursorSetHeldData(&cursor, result_iitem);
            slot->data.item = NULL;
            furnace->recipe_changed = true;
            return;
        } 
        Item* held_item = VCAST_PTR(Item*, held_iitem);
        if (itemEquals(held_item, result_item)) {
            // Held and result item ids mismatch
            return;
        }
        held_item->stack_size += result_item->stack_size;
        VCALL(*result_iitem, destroy);
        slot->data.item = NULL;
        recipeConsumeIngredients(
            furnace->furnace_slots,
            ingredient_consume_sizes,
            slotGroupIndexOffset(FURNACE_INPUT),
            slotGroupIndexOffset(FURNACE_FUEL)
        );
        furnace->recipe_changed = true;
    }
}

InputHandlerState furnaceBlockInputHandler(const Input* input, void* ctx) {
    IBlock* iblock = (IBlock*) ctx;
    FurnaceBlock* furnace = VCAST_PTR(FurnaceBlock*, iblock);
    processFurnaceRecipe(furnace);
    inventoryCursorHandler(
        VCAST_PTR(Inventory*, block_input_handler_context.inventory),
        INVENTORY_SLOT_GROUP_MAIN | INVENTORY_SLOT_GROUP_HOTBAR,
        input
    );
    const PADTYPE* pad = input->pad;
    if (isPressed(pad, BINDING_CURSOR_CLICK)) {
        cursorHandler(furnace, false);
        return INPUT_HANDLER_RETAIN;
    } else if (isPressed(pad, BINDING_DROP_ITEM) && cursor.held_data != NULL) {
        worldDropItemStack(
            world,
            (IItem*) cursor.held_data,
            0
        );
        uiCursorSetHeldData(&cursor, NULL);
    } else if (isPressed(pad, BINDING_SPLIT_OR_STORE_ONE)) {
        cursorHandler(furnace, true);
    }
    if (isPressed(pad, BINDING_OPEN_INVENTORY)) {
        resetBlockRenderUIContext();
        // Leave item in the furnace slots
        return INPUT_HANDLER_RELEASE;
    }
    UNIMPLEMENTED();
    return INPUT_HANDLER_RETAIN;
}

bool furnaceBlockUseAction(VSelf) ALIAS("FurnaceBlock_useAction");
bool FurnaceBlock_useAction(VSelf) {
    VSELF(IBlock);
    furnaceBlockInputHandlerVTable.ctx = self;
    inputSetFocusedHandler(&input, &furnaceBlockInputHandlerVTable);
    block_render_ui_context.function = furnaceBlockRenderUI;
    block_render_ui_context.block = self;
    assetLoadTextureDirect(
        ASSET_BUNDLE__GUI,
        ASSET_TEXTURE__GUI__FURNACE,
        &furnace_texture
    );
    block_render_ui_context.background.texture = &furnace_texture;
    block_render_ui_context.background.texture_coords = vec2_i16(0);
    block_render_ui_context.background.texture_dimensions = vec2_i16(
        FURNACE_TEXTURE_WIDTH,
        FURNACE_TEXTURE_HEIGHT
    );
    block_render_ui_context.background.component = (UIComponent) {
        .position = vec2_i16(
            CENTRE_X - (FURNACE_TEXTURE_WIDTH >> 1),
            CENTRE_Y - (FURNACE_TEXTURE_HEIGHT >> 1)
        ),
        .dimensions = vec2_i16(
            FURNACE_TEXTURE_WIDTH,
            FURNACE_TEXTURE_HEIGHT
        )
    };
    block_render_ui_context.background.tint = vec3_rgb(0x80, 0x80, 0x80);
    block_render_ui_context.background.ot_entry_index = 1;
    return BLOCK_USE_ACTION_CONSUMED;
}

void furnaceRenderTooltip(RenderContext* ctx) {
    if (slotGroupIntersect(FURNACE_INPUT, &cursor.component.position)) {
        const Slot* slot = &furnace_slots[slotGroupCursorSlot(
            FURNACE_INPUT,
            &cursor.component.position
        )];
        if (slot->data.item != NULL) {
            const Item* item = VCAST_PTR(Item*, slot->data.item);
            toolTipRender(ctx, itemGetName(item->id));
        }
    }
    if (slotGroupIntersect(FURNACE_FUEL, &cursor.component.position)) {
        const Slot* slot = &furnace_slots[slotGroupCursorSlot(
            FURNACE_FUEL,
            &cursor.component.position
        )];
        if (slot->data.item != NULL) {
            const Item* item = VCAST_PTR(Item*, slot->data.item);
            toolTipRender(ctx, itemGetName(item->id));
        }
    }
    if (slotGroupIntersect(FURNACE_OUTPUT, &cursor.component.position)) {
        const Slot* slot = &furnace_slots[slotGroupCursorSlot(
            FURNACE_OUTPUT,
            &cursor.component.position
        )];
        if (slot->data.item != NULL) {
            const Item* item = VCAST_PTR(Item*, slot->data.item);
            toolTipRender(ctx, itemGetName(item->id));
        }
    }
}

void furnaceBlockRenderUI(RenderContext* ctx, Transforms* transforms) {
    uiCursorRender(&cursor, ctx, transforms);
    if (quadIntersectLiteral(
        &cursor.component.position,
        CENTRE_X - (FURNACE_TEXTURE_WIDTH >> 1),
        CENTRE_Y - (FURNACE_TEXTURE_HEIGHT >> 1),
        FURNACE_TEXTURE_WIDTH,
        FURNACE_TEXTURE_HEIGHT
    )) {
        furnaceRenderTooltip(ctx);
    }
    inventoryRenderSlots(
        VCAST_PTR(const Inventory*, block_input_handler_context.inventory),
        INVENTORY_SLOT_GROUP_MAIN | INVENTORY_SLOT_GROUP_HOTBAR,
        ctx,
        transforms
    );
    const Slot* slot = &furnace_slots[slotGroupIndexOffset(FURNACE_INPUT)];
    if (slot->data.item != NULL) {
        Item* item = VCAST_PTR(Item*, slot->data.item);
        item->position.vx = slotGroupScreenPosition(FURNACE_INPUT, X, 0);
        item->position.vy = slotGroupScreenPosition(FURNACE_INPUT, Y, 0);
        VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
    }
    slot = &furnace_slots[slotGroupIndexOffset(FURNACE_FUEL)];
    if (slot->data.item != NULL) {
        Item* item = VCAST_PTR(Item*, slot->data.item);
        item->position.vx = slotGroupScreenPosition(FURNACE_FUEL, X, 0);
        item->position.vy = slotGroupScreenPosition(FURNACE_FUEL, Y, 0);
        VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
    }
    slot = &furnace_slots[slotGroupIndexOffset(FURNACE_OUTPUT)];
    if (slot->data.item != NULL) {
        Item* item = VCAST_PTR(Item*, slot->data.item);
        item->position.vx = slotGroupScreenPosition(FURNACE_OUTPUT, X, 0);
        item->position.vy = slotGroupScreenPosition(FURNACE_OUTPUT, Y, 0);
        VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
    }
    uiBackgroundRender(
        &block_render_ui_context.background,
        ctx,
        transforms
    );
}
