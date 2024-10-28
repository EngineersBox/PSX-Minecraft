#include "minecraft.h"

#include <inline_c.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxcd.h>
#include <stdlib.h>

#include "../ui/axis.h"
#include "../structure/primitive/cube.h"
#include "../render/debug.h"
#include "../math/vector.h"
#include "../ui/ui.h"
#include "../entity/player.h"
#include "../resources/assets.h"
#include "../render/font.h"
#include "../util/interface99_extensions.h"
#include "../logging/logging.h"
#include "../util/preprocessor.h"
#include "items/items.h"
#include "items/blocks/item_block_grass.h"
#include "items/blocks/item_block_stone.h"
#include "level/overworld_flatland.h"
#include "level/overworld_perlin.h"
#include "../weather/weather.h"

World* world;
IInputHandler player_handler;
Player* player;

void minecraftInit(VSelf, void* ctx) ALIAS("Minecraft_init");
void Minecraft_init(VSelf, void* ctx) {
    VSELF(Minecraft);
    blocksInitialiseBuiltin();
    itemsInitialiseBuiltin();
    self->internals = (Internals) {
        .ctx = (RenderContext) {
            .active = 0,
            .db = {},
            .primitive = NULL,
            .camera = NULL,
        },
        .transforms = (Transforms) {
            .translation_rotation = vec3_i16_all(0),
            .negative_translation_rotation = vec3_i16_all(0),
            .translation_position = vec3_i32_all(0),
            .negative_translation_position = vec3_i32_all(0),
            .geometry_mtx = {},
            .frustum_mtx = {},
            .lighting_mtx = lighting_direction
        },
        .input = (Input) {},
        .camera = cameraCreate(&self->internals.transforms),
    };
    self->internals.ctx.camera = &self->internals.camera;
    initRenderContext(&self->internals.ctx);
    CdInit();
    inputInit(&self->internals.input);
    // Unpack LZP archive and load assets
    assetsLoad();
    fontLoad();
    // Load font and open a text stream
    FntLoad(960, 256);
    FntOpen(0, 8, 320, 216, 0, 160);
    // Initialise world
    self->world = (World*) malloc(sizeof(World));
    self->world->head.vx = 0;
    self->world->head.vz = 0;
    self->world->centre = vec3_i32_all(0);
    self->world->centre_next = vec3_i32_all(0);
    DYN_PTR(
        &self->world->chunk_provider,
        OverworldPerlinChunkProvider,
        IChunkProvider,
        malloc(sizeof(OverworldPerlinChunkProvider))
    );
    worldInit(self->world, &self->internals.ctx);
    world = self->world;
    // Initialise player
    player = (Player*) malloc(sizeof(Player));
    playerInit(player);
    player->camera = &self->internals.camera;
    const VECTOR player_positon = vec3_i32(0, (CHUNK_BLOCK_SIZE + (BLOCK_SIZE * 2)) << 12, 0);
    iPhysicsObjectSetPosition(&player->entity.physics_object, &player_positon);
    player->entity.physics_object.flags.no_clip = true;
    player_handler = DYN(Player, IInputHandler, player);
    VCALL(player_handler, registerInputHandler, &self->internals.input, world);
    // Register handlers
    VCALL_SUPER(player->inventory, IInputHandler, registerInputHandler, &self->internals.input, NULL);
    VCALL_SUPER(player->hotbar, IInputHandler, registerInputHandler, &self->internals.input, NULL);
    // Initialise camera
    playerUpdateCamera(player);
    cameraUpdate(&self->internals.camera);

    // ==== TESTING: Hotbar ====
    Inventory* inventory = VCAST(Inventory*, player->inventory);
    Slot* slot = inventoryFindFreeSlot(inventory, 0);
    IItem* item = itemCreate();
    GrassItemBlock* grass_item_block = grassItemBlockCreate();
    DYN_PTR(item,GrassItemBlock, IItem, grass_item_block);
    VCALL(*item, init);
    grass_item_block->item_block.item.stack_size = 26;
    inventorySlotSetItem(slot, item);
    VCALL_SUPER(*item, Renderable, applyInventoryRenderAttributes);
    // ==== TESTING: Inventory ====
    slot = &inventory->slots[INVENTORY_SLOT_STORAGE_OFFSET + 2];
    item = itemCreate();
    grass_item_block = grassItemBlockCreate();
    DYN_PTR(item, GrassItemBlock, IItem, grass_item_block);
    VCALL(*item, init);
    grass_item_block->item_block.item.stack_size = 13;
    inventorySlotSetItem(slot, item);
    VCALL_SUPER(*item, Renderable, applyInventoryRenderAttributes);
}

void minecraftCleanup(VSelf) ALIAS("Minecraft_cleanup");
void Minecraft_cleanup(VSelf) {
    VSELF(Minecraft);
    worldDestroy(self->world);
    free(self->world);
    playerDestroy(player);
    free(player);
    assetsFree();
}

void startHandler(Camera* camera);

void minecraftInput(VSelf, const Stats* stats) ALIAS("Minecraft_input");
void Minecraft_input(VSelf, const Stats* stats) {
    VSELF(Minecraft);
    self->internals.camera.mode = 0;
    Input* input = &self->internals.input;
    inputUpdate(input);
}

void minecraftUpdate(VSelf, const Stats* stats) ALIAS("Minecraft_update");
void Minecraft_update(VSelf, const Stats* stats) {
    VSELF(Minecraft);
    worldUpdate(self->world, player, &player->breaking);
    playerUpdate(player, world);
    cameraUpdate(&self->internals.camera);
}

void frustumRenderNormals(const Frustum* frustum, RenderContext* ctx) {
    // Object and light matrix for object
    MATRIX omtx, olmtx;
    // Set object rotation and position
    RotMatrix(&VEC3_I16_ZERO, &omtx);
    TransMatrix(&omtx, &VEC3_I32_ZERO);
    // Multiply light matrix to object matrix
    MulMatrix0(&ctx->camera->transforms->lighting_mtx, &omtx, &olmtx);
    // Set result to GTE light matrix
    gte_SetLightMatrix(&olmtx);
    CompMatrixLV(&ctx->camera->transforms->frustum_mtx, &omtx, &omtx);
    // Save matrix
    PushMatrix();
    // Set matrices
    gte_SetRotMatrix(&omtx);
    gte_SetTransMatrix(&omtx);
    for (int i = 0; i < 6; i++) {
        const Plane* plane = &frustum->planes[i];
        LINE_F2* line = (LINE_F2*) allocatePrimitive(ctx, sizeof(LINE_F2));
        setLineF2(line);
        SVECTOR p0 = vec3_i16(
            plane->point.vx,
            plane->point.vy,
            plane->point.vx
        );
        SVECTOR p1 = vec3_add(
            p0,
            vec3_i16(
                plane->normal.vx * 4,
                plane->normal.vy * 4,
                plane->normal.vz * 4
            )
        );
        gte_ldv01(&p0, &p1);
        gte_rtpt();
        gte_stsxy0(&line->x0);
        gte_stsxy1(&line->x1);
        setRGB0(line, 0xFF, 0x00, 0x00);
        lineF2Render(line, 1, ctx);
    }
    renderCtxUnbindMatrix();
}

void minecraftRender(VSelf, const Stats* stats) ALIAS("Minecraft_render");
void Minecraft_render(VSelf, const Stats* stats) {
    VSELF(Minecraft);
    // Update breaking state textures
    breakingStateUpdateRenderTarget(&player->breaking, &self->internals.ctx);
    // Draw the world
    frustumTransform(&self->internals.ctx.camera->frustum, &self->internals.transforms);
    worldRender(
        self->world,
        (const Player*) &player,
        &self->internals.ctx,
        &self->internals.transforms
    );
    frustumRenderNormals(&self->internals.ctx.camera->frustum, &self->internals.ctx);
    frustumRestore(&self->internals.ctx.camera->frustum);
    if (world->weather.raining || world->weather.storming) {
        weatherRender(
            self->world,
            player,
            &self->internals.ctx,
            &self->internals.transforms
        );
    }
    // Clear window constraints
    renderClearConstraints(&self->internals.ctx);
    // Render UI
    playerRender(player, &self->internals.ctx, &self->internals.transforms);
    /*crosshairDraw(&self->internals.ctx);*/
    axisDraw(
        &self->internals.ctx,
        &self->internals.transforms
    );
    renderClearConstraints(&self->internals.ctx);
    FntPrint(0, PSXMC_VERSION_STRING "\n");
    drawDebugText(stats, &self->internals.camera, world);
    debugDrawPBUsageGraph(
        &self->internals.ctx,
        0,
        SCREEN_YRES - HOTBAR_HEIGHT - 2
    );
    // Flush font to screen
    FntFlush(0);
    // Swap buffers and draw the primitives
    swapBuffers(&self->internals.ctx);
}
