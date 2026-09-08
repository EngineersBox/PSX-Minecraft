#include "block_furnace.h"

#include <psxgpu.h>
#include <psxgte.h>
#include <stdlib.h>

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
#include "../../util/bits.h"
#include "../../util/interface99_extensions.h"
#include "../../util/strings.h"

static Texture furnace_texture = {0};

FWD_DECL Chunk* worldGetChunk(const World* world, const VECTOR* position);
FWD_DECL void worldDropItemStack(World* world, IItem* item, const u8 count);

InputHandlerState furnaceBlockInputHandler(const Input* input, void* ctx);
static InputHandlerVTable furnaceBlockInputHandlerVTable = {
    .ctx = &block_input_handler_context,
    .input_handler = furnaceBlockInputHandler,
    .input_handler_destroy = NULL
};

static RECIPE_PATTERN(pattern, slotGroupSize(FURNACE_INPUT)) = {0};

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
    self->fuel_burn_ticks_start = 0;
    self->recipe = (RecipeSearchResult) {
        .results = (RecipeResults) {0},
        .processing_ticks = 0
    };
    self->recipe_changed = false;
    self->process_recipe = false;
    self->slots[slotGroupIndexOffset(FURNACE_INPUT)] = createSlotInline(FURNACE_INPUT, 0, 0);
    self->slots[slotGroupIndexOffset(FURNACE_FUEL)] = createSlotInline(FURNACE_FUEL, 0, 0);
    self->slots[slotGroupIndexOffset(FURNACE_OUTPUT)] = createSlotInline(FURNACE_OUTPUT, 0, 0);
    self->ingredient_consume_sizes[0] = 0;
    self->ingredient_consume_sizes[1] = 0;
    self->ingredient_consume_sizes[2] = 0;
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

static bool handleFuelConsumption(FurnaceBlock* furnace) {
    DEBUG_LOG("Burn: %d\n", furnace->fuel_burn_ticks);
    if (furnace->fuel_burn_ticks > 0) {
        furnace->process_recipe = true;
        furnace->fuel_burn_ticks--;
    }
    if (furnace->fuel_burn_ticks > 0) return true;
    Slot* slot = &furnace->slots[slotGroupIndexOffset(FURNACE_FUEL)];
    IItem* iitem = slot->data.item;
    DEBUG_LOG("Fuel slot item: %p\n", iitem);
    if (iitem == NULL) {
        furnace->fuel_burn_ticks = 0;
        furnace->fuel_burn_ticks_start = 0;
        furnace->cook_ticks = 0;
        furnace->process_recipe = false;
        return false;
    }
    Item* item = VCAST_PTR(Item*, iitem);
    const u16 item_burnable_ticks = itemGetBurnableTicks(item->id);
    DEBUG_LOG("Item: %d Burnable ticks: %d\n", item->id, item_burnable_ticks);
    if (item_burnable_ticks == 0) {
        furnace->fuel_burn_ticks = 0;
        furnace->fuel_burn_ticks_start = 0;
        furnace->cook_ticks = 0;
        furnace->process_recipe = false;
        return false;
    }
    assert(item->stack_size > 0);
    item->stack_size--;
    furnace->fuel_burn_ticks = item_burnable_ticks;
    furnace->fuel_burn_ticks_start = item_burnable_ticks;
    if (item->stack_size == 0) {
        VCALL(*iitem, destroy);
        slot->data.item = NULL;
    }
    if (furnace->fuel_burn_ticks == 0) {
        furnace->process_recipe = false;
        furnace->cook_ticks = 0;
        return false;
    }
    furnace->process_recipe = true;
    return true;
}

static void handleSmelting(FurnaceBlock* furnace) {
    DEBUG_LOG("Process recipe: %s Cook: %d\n", stringFromBool(furnace->process_recipe), furnace->cook_ticks);
    if (!furnace->process_recipe) return;
    const u16 previous_cook_ticks = furnace->cook_ticks;
    if (furnace->cook_ticks > 0) {
        furnace->cook_ticks--;
    }
    if (previous_cook_ticks != 1 || furnace->recipe.results.result_count == 0) {
        return;
    }
    Slot* slot = &furnace->slots[slotGroupIndexOffset(FURNACE_OUTPUT)];
    const RecipeProcessResult result = recipeProcess(
        &furnace->recipe,
        &slot,
        1,
        true
    );
    switch (result) {
        case RECIPE_PROCESSING_NONE_MATCHING:
        case RECIPE_PROCESSING_INSUFFICIENT_SPACE:
            furnace->process_recipe = false;
            return;
        case RECIPE_PROCESSING_SUCCEEDED:
            furnace->process_recipe = true;
            break;
    }
    recipeConsumeIngredients(
        furnace->slots,
        furnace->ingredient_consume_sizes,
        slotGroupIndexOffset(FURNACE_INPUT),
        slotGroupIndexOffset(FURNACE_FUEL)
    );
    slot = &furnace->slots[slotGroupIndexOffset(FURNACE_INPUT)];
    const Item* item = VCAST_PTR(Item*, slot->data.item);
    if (item != NULL && item->stack_size > 0) {
        furnace->cook_ticks = furnace->recipe.processing_ticks;
        furnace->process_recipe = true;
    } else {
        furnace->process_recipe = false;
    }
}

BlockUpdateResultBitmap furnaceBlockUpdate(VSelf) ALIAS("FurnaceBlock_update");
BlockUpdateResultBitmap FurnaceBlock_update(VSelf) {
    VSELF(FurnaceBlock);
    // DEBUG_LOG("Update furnace block\n");
    const bool burning_fuel = handleFuelConsumption(self);
    handleSmelting(self);
    BlockUpdateResultBitmap bitmap = 0;
    bitmapSetBit(bitmap, BLOCK_UPDATE_RESULT_PERSIST);
    const u8 current_metadata_id = self->block.metadata_id;
    // Block metadata is ordered by direction, left, right,
    // back and front. Each metadata entry has a  burning
    // and non-burning variant, which is indicated by the
    // least-signficant bit of the metadata id.
    self->block.metadata_id = (self->block.orientation - FACE_DIR_LEFT) * 2;
    if (burning_fuel) {
        self->block.metadata_id |= 0b1;
    } else {
        self->block.metadata_id &= ~0b1;
    }
    if (current_metadata_id != self->block.metadata_id) {
        // When the metadata id has changed, we are using a different
        // texture on the block face. So let's trigger a chunk remesh
        // to render that.
        bitmapSetBit(bitmap, BLOCK_UPDATE_RESULT_REMESH_CHUNK);
    }
    return bitmap;
}

static void processFurnaceRecipe(FurnaceBlock* furnace) {
    if (!furnace->recipe_changed) {
        return;
    }
    const Slot* input_slot = &furnace->slots[slotGroupIndexOffset(FURNACE_INPUT)];
    if (input_slot->data.item != NULL) {
        const Item* item = VCAST_PTR(Item*, input_slot->data.item);
        pattern[0] = (RecipePatternEntry) {
            .id = RECIPE_COMPOSITE_ID(item->id, item->metadata_id),
            .stack_size = item->stack_size,
        };
    } else {
        pattern[0] = (RecipePatternEntry) {
            .id = RECIPE_COMPOSITE_ID(ITEMID_AIR, 0),
            .stack_size = 0,
        };
    }
    const RecipeQueryState result = recipeSearch(
        furnace_recipes,
        pattern,
        (Dimension){
            .width = slotGroupDim(FURNACE_INPUT, X),
            .height = slotGroupDim(FURNACE_INPUT, Y)
        },
        &furnace->recipe,
        furnace->ingredient_consume_sizes
    );
    switch (result) {
        case RECIPE_FOUND:
            Slot* output_slot = &furnace->slots[slotGroupIndexOffset(FURNACE_OUTPUT)];
            if (output_slot->data.item == NULL) {
                DEBUG_LOG("Recipe found, output slot empty\n");
                furnace->process_recipe = true;
                break;
            }
            const Item* item = VCAST_PTR(Item*, output_slot->data.item);
            const CompositeID recipe_item_id = furnace->recipe.results.results[0]->item;
            const u8 recipe_stack_size = furnace->recipe.results.results[0]->stack_size;
            furnace->process_recipe = itemIdEqualsExplicit(item->id, item->metadata_id, recipe_item_id.separated.id, recipe_item_id.separated.metadata)
                && item->stack_size + recipe_stack_size < itemGetMaxStackSize(item->id);
            DEBUG_LOG("Recipe found, processing: \n", stringFromBool(furnace->process_recipe));
            break;
        case RECIPE_NOT_FOUND:
            DEBUG_LOG("Recipe not found\n");
            furnace->recipe.results.result_count = 0;
            furnace->recipe.results.results = NULL;
            furnace->recipe.processing_ticks = 0;
            furnace->process_recipe = true;
            break;
    }
    furnace->cook_ticks = 0;
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
        slot = &furnace->slots[slotGroupIndexOffset(FURNACE_INPUT)];
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
        slot = &furnace->slots[slotGroupIndexOffset(FURNACE_FUEL)];
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
    } else if (slotGroupIntersect(FURNACE_OUTPUT, &cursor.component.position) && !split_or_store_one) {
        // NOTE: Don't bother with splitting stacks
        //       since it's a pain the for output slot.
        slot = &furnace->slots[slotGroupIndexOffset(FURNACE_OUTPUT)];
        IItem* result_iitem = slot->data.item;
        if (result_iitem == NULL) {
            return;
        }
        Item* result_item = VCAST_PTR(Item*, result_iitem);
        IItem* held_iitem = (IItem*) cursor.held_data;
        if (held_iitem == NULL) {
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
        const u8 remove_from_stack = min(itemGetMaxStackSize(held_item->id) - held_item->stack_size, result_item->stack_size);
        result_item->stack_size -= remove_from_stack;
        held_item->stack_size += remove_from_stack;
        if (result_item->stack_size == 0) {
            VCALL(*result_iitem, destroy);
            slot->data.item = NULL;
        }
        furnace->recipe_changed = true;
    }
}

InputHandlerState furnaceBlockInputHandler(const Input* input, UNUSED void* ctx) {
    FurnaceBlock* furnace = VCAST_PTR(FurnaceBlock*, block_input_handler_context.block);
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
    return INPUT_HANDLER_RETAIN;
}

bool furnaceBlockUseAction(VSelf) ALIAS("FurnaceBlock_useAction");
bool FurnaceBlock_useAction(VSelf) {
    VSELF(IBlock);
    block_input_handler_context.block = self;
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

void furnaceRenderTooltip(const FurnaceBlock* furnace, RenderContext* ctx) {
    if (slotGroupIntersect(FURNACE_INPUT, &cursor.component.position)) {
        const Slot* slot = &furnace->slots[slotGroupCursorSlot(
            FURNACE_INPUT,
            &cursor.component.position
        )];
        if (slot->data.item != NULL) {
            const Item* item = VCAST_PTR(Item*, slot->data.item);
            toolTipRender(ctx, itemGetName(item->id));
        }
    }
    if (slotGroupIntersect(FURNACE_FUEL, &cursor.component.position)) {
        const Slot* slot = &furnace->slots[slotGroupCursorSlot(
            FURNACE_FUEL,
            &cursor.component.position
        )];
        if (slot->data.item != NULL) {
            const Item* item = VCAST_PTR(Item*, slot->data.item);
            toolTipRender(ctx, itemGetName(item->id));
        }
    }
    if (slotGroupIntersect(FURNACE_OUTPUT, &cursor.component.position)) {
        const Slot* slot = &furnace->slots[slotGroupCursorSlot(
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
    FurnaceBlock* furnace = VCAST_PTR(FurnaceBlock*, block_render_ui_context.block);
    uiCursorRender(&cursor, ctx, transforms);
    if (quadIntersectLiteral(
        &cursor.component.position,
        CENTRE_X - (FURNACE_TEXTURE_WIDTH >> 1),
        CENTRE_Y - (FURNACE_TEXTURE_HEIGHT >> 1),
        FURNACE_TEXTURE_WIDTH,
        FURNACE_TEXTURE_HEIGHT
    )) {
        furnaceRenderTooltip(furnace, ctx);
    }
    inventoryRenderSlots(
        VCAST_PTR(const Inventory*, block_input_handler_context.inventory),
        INVENTORY_SLOT_GROUP_MAIN | INVENTORY_SLOT_GROUP_HOTBAR,
        ctx,
        transforms
    );
    const Slot* slot = &furnace->slots[slotGroupIndexOffset(FURNACE_INPUT)];
    if (slot->data.item != NULL) {
        Item* item = VCAST_PTR(Item*, slot->data.item);
        item->position.vx = slotGroupScreenPosition(FURNACE_INPUT, X, 0);
        item->position.vy = slotGroupScreenPosition(FURNACE_INPUT, Y, 0);
        VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
    }
    slot = &furnace->slots[slotGroupIndexOffset(FURNACE_FUEL)];
    if (slot->data.item != NULL) {
        Item* item = VCAST_PTR(Item*, slot->data.item);
        item->position.vx = slotGroupScreenPosition(FURNACE_FUEL, X, 0);
        item->position.vy = slotGroupScreenPosition(FURNACE_FUEL, Y, 0);
        VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
    }
    slot = &furnace->slots[slotGroupIndexOffset(FURNACE_OUTPUT)];
    if (slot->data.item != NULL) {
        Item* item = VCAST_PTR(Item*, slot->data.item);
        item->position.vx = slotGroupScreenPosition(FURNACE_OUTPUT, X, 0);
        item->position.vy = slotGroupScreenPosition(FURNACE_OUTPUT, Y, 0);
        VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
    }
    if (furnace->fuel_burn_ticks > 0) {
        fixedi32 fire_tex_height = ((fixedi32) furnace->fuel_burn_ticks) * FURNACE_FIRE_TEXTURE_HEIGHT;
        fire_tex_height /= (fixedi32) furnace->fuel_burn_ticks_start;
        POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
        setPolyFT4(pol4);
        setXYWH(
            pol4,
            FURNACE_FIRE_TEXTURE_POS_X,
            FURNACE_FIRE_TEXTURE_POS_Y + (FURNACE_FIRE_TEXTURE_HEIGHT - fire_tex_height),
            FURNACE_FIRE_TEXTURE_WIDTH,
            fire_tex_height
        );
        setUVWH(
            pol4,
            FURNACE_FIRE_TEXTURE_SRC_X,
            FURNACE_FIRE_TEXTURE_SRC_Y + (FURNACE_FIRE_TEXTURE_HEIGHT - fire_tex_height),
            FURNACE_FIRE_TEXTURE_WIDTH,
            fire_tex_height
        );
        setRGB0(pol4, 0x80, 0x80, 0x80);
        pol4->tpage = furnace_texture.tpage;
        pol4->clut = furnace_texture.clut;
        const u32* ot_object = allocateOrderingTable(ctx, 1);
        addPrim(ot_object, pol4);
    }
    if (furnace->cook_ticks) {
        fixedi32 arrow_tex_width = ((fixedi32) furnace->cook_ticks) * FURNACE_ARROW_TEXTURE_WIDTH;
        arrow_tex_width /= (fixedi32) furnace->recipe.processing_ticks;
        POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
        setPolyFT4(pol4);
        setXYWH(
            pol4,
            FURNACE_ARROW_TEXTURE_POS_X,
            FURNACE_ARROW_TEXTURE_POS_Y,
            arrow_tex_width,
            FURNACE_ARROW_TEXTURE_HEIGHT
        );
        setUVWH(
            pol4,
            FURNACE_ARROW_TEXTURE_SRC_X,
            FURNACE_ARROW_TEXTURE_SRC_Y,
            arrow_tex_width,
            FURNACE_ARROW_TEXTURE_HEIGHT
        );
        setRGB0(pol4, 0x80, 0x80, 0x80);
        pol4->tpage = furnace_texture.tpage;
        pol4->clut = furnace_texture.clut;
        const u32* ot_object = allocateOrderingTable(ctx, 1);
        addPrim(ot_object, pol4);
    }
    uiBackgroundRender(
        &block_render_ui_context.background,
        ctx,
        transforms
    );
}
