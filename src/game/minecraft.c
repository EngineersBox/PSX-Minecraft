#include "minecraft.h"

#include <inline_c.h>
#include <psxgpu.h>
#include <psxgte.h>

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
#include "stdlib.h"

// Reference texture data
extern const uint32_t tim_texture[];

#define MARKER_SIZE 20
SVECTOR verts[8] = {
    { -MARKER_SIZE, -MARKER_SIZE, -MARKER_SIZE, 0 },
    {  MARKER_SIZE, -MARKER_SIZE, -MARKER_SIZE, 0 },
    { -MARKER_SIZE,  MARKER_SIZE, -MARKER_SIZE, 0 },
    {  MARKER_SIZE,  MARKER_SIZE, -MARKER_SIZE, 0 },
    {  MARKER_SIZE, -MARKER_SIZE,  MARKER_SIZE, 0 },
    { -MARKER_SIZE, -MARKER_SIZE,  MARKER_SIZE, 0 },
    {  MARKER_SIZE,  MARKER_SIZE,  MARKER_SIZE, 0 },
    { -MARKER_SIZE,  MARKER_SIZE,  MARKER_SIZE, 0 }
};

World* world;
Camera camera;
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
            .primitive = NULL
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
        .camera = {}
    };
    camera = cameraCreate(&self->internals.transforms);
    DYN_PTR(&self->internals.camera, Camera, IInputHandler, &camera);
    self->internals.ctx.camera = VCAST(Camera*, self->internals.camera);
    gte_SetBackColor(
        back_colour.r,
        back_colour.g,
        back_colour.b
    );
    gte_SetFarColor(
        far_colour.r,
        far_colour.g,
        far_colour.b
    );
    // Set light ambient color and light colorma trix
    gte_SetColorMatrix(&lighting_colour);
    // FOV?
    gte_SetGeomScreen(100);
    initRenderContext(&self->internals.ctx);
    inputInit(&self->internals.input);
    // Unpack LZP archive and load assets
    assetsLoad();
    fontLoad();
    // Load font and open a text stream
    FntLoad(960, 0);
    FntOpen(0, 8, 320, 216, 0, 160);
    FntOpen(
        SCREEN_XRES >> 1,
        8,
        SCREEN_XRES >> 1,
        216,
        0,
        (SCREEN_XRES / FONT_SPRITE_WIDTH) * 6
    );
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
    iPhysicsObjectSetPosition(&player->physics_object, &player_positon);
    player->physics_object.flags.no_clip = true;
    player_handler = DYN(Player, IInputHandler, player);
    VCALL(player_handler, registerInputHandler, &self->internals.input, world);
    // Register handlers
    VCALL(*player->camera, registerInputHandler, &self->internals.input, NULL);
    VCALL_SUPER(player->inventory, IInputHandler, registerInputHandler, &self->internals.input, NULL);
    VCALL_SUPER(player->hotbar, IInputHandler, registerInputHandler, &self->internals.input, NULL);

    // Initialise camera
    playerUpdateCamera(player);
    DEBUG_LOG(
        "Player pos: " VEC_PATTERN " Camera pos: " VEC_PATTERN "\n",
        VEC_LAYOUT(player->physics_object.position),
        VEC_LAYOUT(camera.position)
    );

    // ==== TESTING: Hotbar ====
    Inventory* inventory = VCAST(Inventory*, player->inventory);
    Slot* slot = inventoryFindFreeSlot(inventory, 0);
    IItem* item = itemCreate();
    GrassItemBlock* grass_item_block = grassItemBlockCreate();
    DYN_PTR(item, GrassItemBlock, IItem, grass_item_block);
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
    Camera* camera = VCAST(Camera*, self->internals.camera);
    camera->mode = 0;
    Input* input = &self->internals.input;
    inputUpdate(input);
}

void minecraftUpdate(VSelf, const Stats* stats) ALIAS("Minecraft_update");
void Minecraft_update(VSelf, const Stats* stats) {
    VSELF(Minecraft);
    worldUpdate(self->world, player, &player->breaking);
    playerUpdate(player, world);
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
        &self->internals.ctx,
        &self->internals.transforms
    );
    frustumRestore(&self->internals.ctx.camera->frustum);
    // Clear window constraints
    renderClearConstraints(&self->internals.ctx);
    // Render UI
    playerRender(player, &self->internals.ctx, &self->internals.transforms);
    // crosshairDraw(&render_context);
    axisDraw(&self->internals.ctx, &self->internals.transforms, &camera);
    FntPrint(0, PSXMC_VERSION_STRING "\n");
    drawDebugText(stats, &camera, world);
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
