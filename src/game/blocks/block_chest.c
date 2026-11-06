#include "block_chest.h"

#include "block.h"
#include "block_id.h"
#include "../gui/inventory.h"
#include "../gui/tooltip.h"
#include "../gui/utils.h"
#include "../items/blocks/item_block_chest.h"
#include "../items/items.h"
#include "../world/world_structure.h"
#include "../../core/input/input.h"
#include "../../ui/components/cursor.h"
#include "../../util/bits.h"
#include "../../util/interface99_extensions.h"
#include "../../util/memory.h"
#include "psxgte.h"

FWD_DECL void worldDropItemStack(World* world, IItem* item, const u8 count);
FWD_DECL IBlock* worldGetBlock(const World* world, const VECTOR position);

static Texture chest_texture = {0};
Timestamp chest_debounce = 0;

InputHandlerState chestBlockInputHandler(const Input* input, void* ctx);
static VECTOR chest_world_pos = vec3_i32(0);
static InputHandlerVTable chestBlockInputHandlerVTable = {
    .ctx = &block_input_handler_context,
    .input_handler = chestBlockInputHandler,
    .input_handler_destroy = NULL
};

DEFN_BLOCK_CONSTRUCTOR_IMPL_STATEFUL(chest) {
    if (from_item != NULL) {
        Item* item = VCAST_PTR(Item*, from_item);
        item->stack_size--;
    }
    IBlock* iblock = iblockCreate();
    ChestBlock* chest_block = malloc(sizeof(ChestBlock));
    zeroed(chest_block);
    DYN_PTR(iblock, ChestBlock, IBlock, chest_block);
    VCALL(*iblock, init);
    return iblock;
}

void chestBlockInit(VSelf) ALIAS("ChestBlock_init");
void ChestBlock_init(VSelf) {
    VSELF(ChestBlock);
    self->block = declareBlock(BLOCKID_CHEST);
    for (int y = 0; y < slotGroupDim(CHEST_SINGLE, Y); y++) {
        for (int x = 0; x < slotGroupDim(CHEST_SINGLE, X); x++) {
            self->slots[(y * slotGroupDim(CHEST_SINGLE, X)) + x] = createSlotInline(CHEST_SINGLE, x, y);
        }
    }
}

BlockUpdateResultBitmap chestBlockUpdate(VSelf, const VECTOR block_world_pos) ALIAS("ChestBlock_update");
BlockUpdateResultBitmap ChestBlock_update(VSelf, const VECTOR block_world_pos) {
    // NOTE: This is only triggered once when the block is placed after
    //       initialisation and stuff has completed. Purpose is only to
    //       update the single/double state and apply necessary changes.
    VSELF(ChestBlock);
    ChestBlock* other = NULL;
    SVECTOR other_relative_pos = vec3_i16(0);
    FaceDirection other_relative_dir = FACE_DIR_LEFT;
    for (FaceDirection dir = FACE_DIR_LEFT; dir <= FACE_DIR_FRONT; dir++) {
        other_relative_pos = WORLD_FACE_DIRECTION_NORMALS[dir];
        const VECTOR query_pos = vec3_add(
            block_world_pos,
            other_relative_pos
        );
        const IBlock* iblock = worldGetBlock(world, query_pos);
        if (iblock == NULL) continue;
        const Block* block = VCAST_PTR(Block*, iblock);
        if (block->id != BLOCKID_CHEST) continue;
        other = VCAST_PTR(ChestBlock*, iblock);
        other_relative_dir = dir;
    }
    if (other == NULL) {
        self->type = CHEST_TYPE_SINGLE;
        return 0;
    }
    if (faceDirectionIsSameAxis(self->block.orientation, other->block.orientation)
        && faceDirectionIsSameAxis(other->block.orientation, other_relative_dir)) {
        const SVECTOR adjusted_dir= FACE_DIRECTION_RELATIVE_RIGHT[other_relative_dir];
        self->block.orientation = faceDirectionClosestNormal(vec3_as(VECTOR, adjusted_dir));
        other->block.orientation = self->block.orientation;
    } else if (faceDirectionIsSameAxis(self->block.orientation, other_relative_dir)) {
        self->block.orientation = other->block.orientation;
    } else {
        other->block.orientation = self->block.orientation;
    }
    if (vec3_equal(other_relative_pos, FACE_DIRECTION_RELATIVE_RIGHT[self->block.orientation])) {
        self->type = CHEST_TYPE_DOUBLE_LEFT;
        other->type = CHEST_TYPE_DOUBLE_RIGHT;
    } else {
        self->type = CHEST_TYPE_DOUBLE_RIGHT;
        other->type = CHEST_TYPE_DOUBLE_LEFT;
    }
    // FIXME: If the chest is between chunks, this won't trigger a remesh
    //        in the other chunk
    BlockUpdateResultBitmap result = 0;
    bitmapSetBit(result, BLOCK_UPDATE_RESULT_REMESH_CHUNK);
    return result;
}

IItem* chestBlockDestroy(VSelf,
                         bool drop_item,
                         const VECTOR block_world_pos) ALIAS("ChestBlock_destroy");
IItem* ChestBlock_destroy(VSelf,
                          bool drop_item,
                          const VECTOR block_world_pos) {
    VSELF(ChestBlock);
    IBlock* iblock = NULL;
    ChestBlock* other_chest_half = NULL;
    switch (self->type) {
        case CHEST_TYPE_SINGLE:
            break;
        case CHEST_TYPE_DOUBLE_LEFT:
            iblock = worldGetBlock(world, vec3_add(
                block_world_pos,
                FACE_DIRECTION_RELATIVE_LEFT[self->block.orientation]
            ));
            other_chest_half = VCAST_PTR(ChestBlock*, iblock);
            other_chest_half->type = CHEST_TYPE_SINGLE;
            break;
        case CHEST_TYPE_DOUBLE_RIGHT:
            iblock = worldGetBlock(world, vec3_add(
                block_world_pos,
                FACE_DIRECTION_RELATIVE_RIGHT[self->block.orientation]
            ));
            other_chest_half = VCAST_PTR(ChestBlock*, iblock);
            other_chest_half->type = CHEST_TYPE_SINGLE;
            break;
    }
    for (int i = 0; i < slotGroupSize(CHEST_SINGLE); i++) {
        IItem* iitem = self->slots[i].data.item;
        if (iitem == NULL) continue;
        worldDropItemStack(world, iitem, 0);
        Item* item = VCAST_PTR(Item*, iitem);
        item->world_entity->physics_object.position = vec3_const_mul(vec3_i32(
            block_world_pos.vx,
            -block_world_pos.vy,
            block_world_pos.vz
        ), ONE_BLOCK);
        self->slots[i].data.item = NULL;
    }
    return drop_item ? chestBlockProvideItem(self) : NULL;
}

IItem* chestBlockProvideItem(VSelf) ALIAS("ChestBlock_provideItem");
IItem* ChestBlock_provideItem(VSelf) {
    VSELF(ChestBlock);
    IItem* item = itemConstructor(chest)(0);
    ChestItemBlock* item_block = VCAST_PTR(ChestItemBlock*, item);
    itemBlockReplicateFaceAttributes(item_block->item_block, self->block);
    item_block->item_block.item.stack_size = 1;
    item_block->item_block.item.bob_direction = 1;
    return item;
}

static void cursorHandler(ChestBlock* chest,
                          const bool split_or_store_one) {
    if (!debounce(&chest_debounce, CHEST_DEBOUNCE_MS)) {
        return;
    }
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
            0 // Implies all in stack
        );
        uiCursorSetHeldData(&cursor, NULL);
        return;
    }
    Slot* slot = NULL;
    IBlock* iblock = NULL;
    ChestBlock* other_chest = NULL;
    switch (chest->type) {
        case CHEST_TYPE_SINGLE:
            if (!slotGroupIntersect(
                CHEST_SINGLE,
                &cursor.component.position
            )) {
                break;
            }
            slot = &chest->slots[slotGroupCursorSlot(
                CHEST_SINGLE,
                &cursor.component.position
            )];
            break;
        case CHEST_TYPE_DOUBLE_LEFT:
            if (slotGroupIntersect(
                CHEST_DOUBLE_LEFT,
                &cursor.component.position
            )) {
                slot = &chest->slots[slotGroupCursorSlot(
                    CHEST_DOUBLE_LEFT,
                    &cursor.component.position
                )];
                break;
            }
            iblock = worldGetBlock(world, vec3_add(
                chest_world_pos,
                FACE_DIRECTION_RELATIVE_LEFT[chest->block.orientation]
            ));
            other_chest = VCAST_PTR(ChestBlock*, iblock);
            if (slotGroupIntersect(
                CHEST_DOUBLE_RIGHT,
                &cursor.component.position
            )) {
                slot = &other_chest->slots[slotGroupCursorSlot(
                    CHEST_DOUBLE_RIGHT,
                    &cursor.component.position
                )];
            }
            break;
        case CHEST_TYPE_DOUBLE_RIGHT:
            if (slotGroupIntersect(
                CHEST_DOUBLE_RIGHT,
                &cursor.component.position
            )) {
                slot = &chest->slots[slotGroupCursorSlot(
                    CHEST_DOUBLE_RIGHT,
                    &cursor.component.position
                )];
                break;
            }
            iblock = worldGetBlock(world, vec3_add(
                chest_world_pos,
                FACE_DIRECTION_RELATIVE_LEFT[chest->block.orientation]
            ));
            other_chest = VCAST_PTR(ChestBlock*, iblock);
            if (slotGroupIntersect(
                CHEST_DOUBLE_LEFT,
                &cursor.component.position
            )) {
                slot = &other_chest->slots[slotGroupCursorSlot(
                    CHEST_DOUBLE_LEFT,
                    &cursor.component.position
                )];
            }
            break;
    }
    if (slot == NULL) return;
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
}

InputHandlerState chestBlockInputHandler(const Input* input, UNUSED void* ctx) {
    ChestBlock* chest = (ChestBlock*) block_input_handler_context.block;
    inventoryCursorHandler(
        VCAST_PTR(Inventory*, block_input_handler_context.inventory),
        INVENTORY_SLOT_GROUP_MAIN | INVENTORY_SLOT_GROUP_HOTBAR,
        input
    );
    const PADTYPE* pad = input->pad;
    if (isPressed(pad, BINDING_CURSOR_CLICK)) {
        cursorHandler(chest, false);
    } else if (isPressed(pad, BINDING_SPLIT_OR_STORE_ONE)) {
        cursorHandler(chest, true);
    }
    if (isPressed(pad, BINDING_DROP_ITEM) && cursor.held_data != NULL) {
        worldDropItemStack(
            world,
            (IItem*) cursor.held_data,
            0
        );
        uiCursorSetHeldData(&cursor, NULL);
    }
    if (isPressed(pad, BINDING_OPEN_INVENTORY)) {
        resetBlockRenderUIContext();
        // Leave item in the furnace slots
        return INPUT_HANDLER_RELEASE;
    }
    return INPUT_HANDLER_RETAIN;
}

INLINE static void chestSingleRenderUI(const ChestBlock* chest,
                                       RenderContext* ctx,
                                       Transforms* transforms) {
    for (int y = 0; y < slotGroupDim(CHEST_SINGLE, Y); y++) {
        for (int x = 0; x < slotGroupDim(CHEST_SINGLE, X); x++) {
            const Slot* slot = &chest->slots[
                slotGroupIndexOffset(CHEST_SINGLE)
                + (y * slotGroupDim(CHEST_SINGLE, X)) + x
            ];
            if (slot->data.item == NULL) continue;
            Item* item = VCAST_PTR(Item*, slot->data.item);
            item->position.vx = slotGroupScreenPosition(CHEST_SINGLE, X, x);
            item->position.vy = slotGroupScreenPosition(CHEST_SINGLE, Y, y);
            VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
        }
    }
    if (slotGroupIntersect(CHEST_SINGLE, &cursor.component.position)) {
        const Slot* slot = &chest->slots[slotGroupCursorSlot(
            CHEST_SINGLE,
            &cursor.component.position
        )];
        if (slot->data.item != NULL) {
            const Item* item = VCAST_PTR(Item*, slot->data.item);
            toolTipRender(ctx, itemGetName(item->id));
        }
    }
}

INLINE static void chestDoubleLeftRenderUI(const ChestBlock* chest,
                                           RenderContext* ctx,
                                           Transforms* transforms) {
    for (int y = 0; y < slotGroupDim(CHEST_DOUBLE_LEFT, Y); y++) {
        for (int x = 0; x < slotGroupDim(CHEST_DOUBLE_LEFT, X); x++) {
            const Slot* slot = &chest->slots[
                slotGroupIndexOffset(CHEST_DOUBLE_LEFT)
                + (y * slotGroupDim(CHEST_DOUBLE_LEFT, X)) + x
            ];
            if (slot->data.item == NULL) continue;
            Item* item = VCAST_PTR(Item*, slot->data.item);
            item->position.vx = slotGroupScreenPosition(CHEST_DOUBLE_LEFT, X, x);
            item->position.vy = slotGroupScreenPosition(CHEST_DOUBLE_LEFT, Y, y);
            VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
        }
    }
    if (slotGroupIntersect(CHEST_DOUBLE_LEFT, &cursor.component.position)) {
        const Slot* slot = &chest->slots[slotGroupCursorSlot(
            CHEST_DOUBLE_LEFT,
            &cursor.component.position
        )];
        if (slot->data.item != NULL) {
            const Item* item = VCAST_PTR(Item*, slot->data.item);
            toolTipRender(ctx, itemGetName(item->id));
        }
    }
}

INLINE static void chestDoubleRightRenderUI(const ChestBlock* chest,
                                            RenderContext* ctx,
                                            Transforms* transforms) {
    for (int y = 0; y < slotGroupDim(CHEST_DOUBLE_RIGHT, Y); y++) {
        for (int x = 0; x < slotGroupDim(CHEST_DOUBLE_RIGHT, X); x++) {
            const Slot* slot = &chest->slots[
                slotGroupIndexOffset(CHEST_DOUBLE_RIGHT)
                + (y * slotGroupDim(CHEST_DOUBLE_RIGHT, X)) + x 
            ];
            if (slot->data.item == NULL) continue;
            Item* item = VCAST_PTR(Item*, slot->data.item);
            item->position.vx = slotGroupScreenPosition(CHEST_DOUBLE_RIGHT, X, x);
            item->position.vy = slotGroupScreenPosition(CHEST_DOUBLE_RIGHT, Y, y);
            VCALL_SUPER(*slot->data.item, Renderable, renderInventory, ctx, transforms);
        }
    }
    if (slotGroupIntersect(CHEST_DOUBLE_RIGHT, &cursor.component.position)) {
        const Slot* slot = &chest->slots[slotGroupCursorSlot(
            CHEST_DOUBLE_RIGHT,
            &cursor.component.position
        )];
        if (slot->data.item != NULL) {
            const Item* item = VCAST_PTR(Item*, slot->data.item);
            toolTipRender(ctx, itemGetName(item->id));
        }
    }
}

void chestBlockRenderUI(RenderContext* ctx, Transforms* transforms) {
    const ChestBlock* chest = (ChestBlock*) block_render_ui_context.block;
    uiCursorRender(&cursor, ctx, transforms);
    inventoryRenderSlots(
        VCAST_PTR(const Inventory*, block_input_handler_context.inventory),
        INVENTORY_SLOT_GROUP_MAIN | INVENTORY_SLOT_GROUP_HOTBAR,
        ctx,
        transforms
    );
    const ChestBlock* left = NULL;
    const ChestBlock* right = NULL;
    const IBlock* iblock = NULL;
    switch (chest->type) {
        case CHEST_TYPE_SINGLE:
            chestSingleRenderUI(chest, ctx, transforms);
            return;
        case CHEST_TYPE_DOUBLE_LEFT:
            left = chest;
            iblock = worldGetBlock(
                world,
                vec3_add(
                    chest_world_pos,
                    FACE_DIRECTION_RELATIVE_LEFT[chest->block.orientation]
                )
            );
            right = VCAST_PTR(ChestBlock*, iblock);
            break;
        case CHEST_TYPE_DOUBLE_RIGHT:
            iblock = worldGetBlock(
                world,
                vec3_add(
                    chest_world_pos,
                    FACE_DIRECTION_RELATIVE_RIGHT[chest->block.orientation]
                )
            );
            left = VCAST_PTR(ChestBlock*, iblock);
            right = chest;
            break;
    }
    chestDoubleLeftRenderUI(left, ctx, transforms);
    chestDoubleRightRenderUI(right, ctx, transforms);
    uiBackgroundRender(
        &block_render_ui_context.background,
        ctx,
        transforms
    );
}

bool chestBlockUseAction(VSelf, const VECTOR block_world_pos) ALIAS("ChestBlock_useAction");
bool ChestBlock_useAction(VSelf, const VECTOR block_world_pos) {
    VSELF(ChestBlock);
    block_input_handler_context.block = (Block*) self;
    chest_world_pos = block_world_pos;
    inputSetFocusedHandler(&input, &chestBlockInputHandlerVTable);
    block_render_ui_context.function = chestBlockRenderUI;
    block_render_ui_context.block = (Block*) self;
    i16 w;
    i16 h;
    i16 y_offset;
    int file_index;
    if (self->type == CHEST_TYPE_SINGLE) {
        w = CHEST_SINGLE_TEXTURE_WIDTH;
        h = CHEST_SINGLE_TEXTURE_HEIGHT;
        y_offset = 0;
        file_index = ASSET_TEXTURE__GUI__CHEST;
    } else {
        w = CHEST_DOUBLE_TEXTURE_WIDTH;
        h = CHEST_DOUBLE_TEXTURE_HEIGHT;
        y_offset = CHEST_DOUBLE_TEXTURE_OFFSET_Y;
        file_index = ASSET_TEXTURE__GUI__DOUBLE_CHEST;
    }
    assetLoadTextureDirect(
        ASSET_BUNDLE__GUI,
        file_index,
        &chest_texture
    );
    block_render_ui_context.background.texture = &chest_texture;
    block_render_ui_context.background.texture_coords = vec2_i16(0);
    block_render_ui_context.background.texture_dimensions = vec2_i16(w, h);
    block_render_ui_context.background.component = (UIComponent) {
        .position = vec2_i16(
            CENTRE_X - (w >> 1),
            CENTRE_Y - (h >> 1) - y_offset
        ),
        .dimensions = vec2_i16(w, h)
    };
    block_render_ui_context.background.tint = vec3_rgb(0x80, 0x80, 0x80);
    block_render_ui_context.background.ot_entry_index = 1;
    return BLOCK_USE_ACTION_CONSUMED;
}
