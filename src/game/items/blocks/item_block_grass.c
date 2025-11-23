#include "item_block_grass.h"

#include <interface99.h>
#include "../../../core/std/stdlib.h"

#include "../../../util/interface99_extensions.h"
#include "../../../util/memory.h"
#include "../item_id.h"
#include "item_block.h"

GrassItemBlock* grassItemBlockCreate() {
    GrassItemBlock* itemblock = malloc(sizeof(GrassItemBlock));
    zeroed(itemblock);
    return itemblock;
}

DEFN_ITEM_CONSTRUCTOR(grass) {
    IItem* item = itemCreate();
    GrassItemBlock* grass_item_block = grassItemBlockCreate();
    grass_item_block->item_block.item.metadata_id = 0;
    DYN_PTR(item, GrassItemBlock, IItem, grass_item_block);
    VCALL(*item, init);
    return item;
}

void grassItemBlockDestroy(VSelf) ALIAS("GrassItemBlock_destroy");
void GrassItemBlock_destroy(VSelf) {
    VSELF(GrassItemBlock);
    free(self);
}

void grassItemBlockRenderWorld(VSelf,
                               const Chunk* chunk,
                               RenderContext* ctx,
                               Transforms* transforms) ALIAS("GrassItemBlock_renderWorld");
void GrassItemBlock_renderWorld(VSelf,
                                const Chunk* chunk,
                                RenderContext* ctx,
                                Transforms* transforms) {
    VSELF(GrassItemBlock);
    itemBlockRenderWorld(&self->item_block, chunk, ctx, transforms);
}

void grassItemBlockRenderInventory(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("GrassItemBlock_renderInventory");
void GrassItemBlock_renderInventory(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(GrassItemBlock);
    itemBlockRenderInventory(&self->item_block, ctx, transforms);
}

void grassItemBlockRenderHand(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("GrassItemBlock_renderHand");
void GrassItemBlock_renderHand(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(GrassItemBlock);
    itemBlockRenderHand(&self->item_block, ctx, transforms);
}

void grassItemBlockInit(VSelf) ALIAS("GrassItemBlock_init");
void GrassItemBlock_init(VSelf) {
    VSELF(GrassItemBlock);
    self->item_block = (ItemBlock) {
        .item = declareItem(ITEMID_GRASS),
        .face_attributes = { declareTintedFaceAttributes(
            2 /*49*/, NO_TINT,
            0 /*49*/, /*NO_TINT,*/ faceTint(91, 139, 50, 1),
            3 /*49*/, NO_TINT,
            3 /*49*/, NO_TINT,
            3 /*49*/, NO_TINT,
            3 /*49*/, NO_TINT
        ) }
    };
}
