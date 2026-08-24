#include "block_furnace.h"

#include "block.h"
#include "block_id.h"
#include "../items/blocks/item_block_furnace.h"
#include "../items/items.h"
#include "../gui/inventory.h"
#include "../gui/tooltip.h"
#include "../gui/utils.h"
#include "../recipe/crafting.h"
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

InputHandlerState furnaceBlockInputHandler(const Input* input, void* ctx);
static InputHandlerVTable furnaceBlockInputHandlerVTable = {
    .ctx = &block_input_handler_context,
    .input_handler = furnaceBlockInputHandler,
    .input_handler_destroy = NULL
};

static u8 ingredient_consume_sizes[slotGroupSize(FURNACE_INPUT) + slotGroupSize(FURNACE_FUEL)] = {0};
static RECIPE_PATTERN(pattern, slotGroupSize(FURNACE_INPUT) + slotGroupSize(FURNACE_FUEL)) = {0};

DEFN_BLOCK_CONSTRUCTOR_IMPL_STATEFUL(furnace) {
    TODO("Constructor for furnace block");
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
    UNIMPLEMENTED();
}


bool furnaceBlockUseAction(VSelf) ALIAS("FurnaceBlock_useAction");
bool FurnaceBlock_useAction(VSelf) {
    VSELF(IBlock);
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
