#include "block_furnace.h"

#include "block.h"
#include "block_id.h"
#include "../items/blocks/item_block_furnace.h"
#include "../../core/input/input.h"
#include "../../logging/logging.h"
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
    UNIMPLEMENTED();
    return false;
}

void furnaceBlockRenderUI(RenderContext* ctx, Transforms* transforms) {
}
