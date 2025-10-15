#include "minecraft.h"

#include <inline_c.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxcd.h>
#include "../core/std/stdlib.h"
/*#include <sys/ioctl.h>*/
/*#include <sys/fcntl.h>*/
/*#include <psxsio.h>*/

#include "../ui/axis.h"
#include "../render/debug.h"
#include "../math/vector.h"
#include "../ui/ui.h"
#include "entity/player.h"
#include "../resources/assets.h"
#include "../render/duration_tree.h"
#include "../render/font.h"
#include "../util/interface99_extensions.h"
#include "../logging/logging.h"
#include "../util/preprocessor.h"
#include "blocks/block.h"
#include "gui/inventory.h"
#include "gui/slot.h"
#include "items/blocks/item_block.h"
#include "items/item_id.h"
#include "items/items.h"
#include "world/level/overworld_perlin.h"
#include "weather/weather.h"
// #include "../core/console.h"
#include "../debug/debug_defines.h"
#include "../structure/primitive/primitive.h"

World* world;
IInputHandler player_handler;
IEntity player_entity;
Player* player;

/*int helpHandler(const Console* console, char* tokens[CONSOLE_MAX_TOKENS]) {*/
/*    DEBUG_LOG("[CONSOLE] Invoked help handler: %s\n", tokens[0]);*/
/*    return 0;*/
/*}*/
/**/
/*Console* console;*/
/*int console_commands_size = 1;*/
/*ConsoleCommand console_commands[1] = {*/
/*    [0]=(ConsoleCommand) {*/
/*        .name = "help",*/
/*        .handler = helpHandler*/
/*    }*/
/*};*/

void renderLoadingScreen(RenderContext* ctx, Transforms* transforms) {

    // Change to white clear colour
    setRGB0(
        &ctx->db[ctx->active].draw_env,
        0xFF,
        0xFF,
        0xFF
    );
    ctx->db[1].draw_env.isbg = 1; // Background
    ctx->db[1].draw_env.dtd = 1; // Dithered
    // Apply drawing environment to first double buffer
    PutDrawEnv(&ctx->db[ctx->active].draw_env);
    // Draw logo
    POLY_FT4* pol4 = (POLY_FT4*) allocatePrimitive(ctx, sizeof(POLY_FT4));
    setXYWH(
        pol4,
        CENTRE_X - (128 >> 1),
        CENTRE_Y - (128 >> 1),
        128,
        128
    );
    setRGB0(pol4, 0xFF, 0xFF, 0xFF);
    setUVWH(pol4, 0, 0, 128, 128);
    Texture mojang_logo = {0};
    assetLoadTextureDirect(
        ASSET_BUNDLE__START,
        ASSET_TEXTURE__START__MOJANG,
        &mojang_logo
    );
    swapBuffers(ctx);
    pol4->tpage = mojang_logo.tpage;
    pol4->clut = mojang_logo.clut;
    polyFT4Render(pol4, 1, ctx);
    renderClearConstraintsIndex(ctx, 1);
    swapBuffers(ctx);
    pcsx_debugbreak();
}

void minecraftInit(VSelf, void* ctx) ALIAS("Minecraft_init");
void Minecraft_init(VSelf, UNUSED void* ctx) {
    VSELF(Minecraft);
    self->ctx = (RenderContext) {
        .active = 0,
        .db = {},
        .primitive = NULL,
        .camera = NULL,
    };
    self->transforms = (Transforms) {
        .translation_rotation = vec3_i16(0),
        .negative_translation_rotation = vec3_i16(0),
        .translation_position = vec3_i32(0),
        .negative_translation_position = vec3_i32(0),
        .geometry_mtx = mat4_i16_i32(0),
        .frustum_mtx = mat4_i16_i32(0),
        .lighting_mtx = lighting_direction
    };
    self->camera = cameraCreate(&self->transforms),
    self->ctx.camera = &self->camera;
    initRenderContext(&self->ctx);
    // Input initialisation needs to be before block init
    // since some blocks need to register handlers
    /*AddSIO(0x1c200);*/
    CdInit();
    inputInit(&input);
    renderLoadingScreen(&self->ctx, &self->transforms);
    // Unpack LZP archive and load assets
    assetsLoad();
    fontLoad();
    // Load font and open a text stream
    FntLoad(960, 256);
    FntOpen(0, 8, 320, 216, 0, 160);
    // Debugging
    durationTreesInit();
    // Initialise game elements
    blocksInitialiseBuiltin();
    itemsInitialiseBuiltin();
    // Initialise world
    world = worldNew();
    DYN_PTR(
        &world->chunk_provider,
        OverworldPerlinChunkProvider,
        IChunkProvider,
        malloc(sizeof(OverworldPerlinChunkProvider))
    );
    worldInit(world, &self->ctx);
    // Initialise player
    player = playerNew();
    DYN_PTR(&player_entity, Player, IEntity, player);
    playerInit(player);
    block_input_handler_context.inventory = &player->inventory;
    player->camera = &self->camera;
    const VECTOR player_positon = vec3_i32(
        0,
        (CHUNK_BLOCK_SIZE + (BLOCK_SIZE << 1)) << FIXED_POINT_SHIFT,
        0
    );
    iPhysicsObjectSetPosition(&player->entity.physics_object, &player_positon);
#if isDebugTagEnabled(PLAYER_NOCLIP)
    player->entity.physics_object.flags.no_clip = true;
#else
    player->entity.physics_object.flags.no_clip = false;
#endif
    player_handler = DYN(Player, IInputHandler, player);
    VCALL(player_handler, registerInputHandler, &input, world);
    // Register handlers
    VCALL_SUPER(player->inventory, IInputHandler, registerInputHandler, &input, NULL);
    VCALL_SUPER(player->hotbar, IInputHandler, registerInputHandler, &input, NULL);
    // Initialise camera
    playerUpdateCamera(player);
    cameraUpdate(&self->camera);

    // ==== TESTING: Hotbar ====
    Inventory* inventory = VCAST(Inventory*, player->inventory);
    /*Slot* slot = inventoryFindFreeSlot(inventory, 0);*/
    /*IItem* item = itemCreate();*/
    /*GrassItemBlock* grass_item_block = grassItemBlockCreate();*/
    /*DYN_PTR(item,GrassItemBlock, IItem, grass_item_block);*/
    /*VCALL(*item, init);*/
    /*grass_item_block->item_block.item.stack_size = 26;*/
    /*inventorySlotSetItem(slot, item);*/
    /*VCALL_SUPER(*item, Renderable, applyInventoryRenderAttributes);*/
    // ==== TESTING: Inventory ====
    /*slot = &inventory->slots[slotGroupIndexOffset(INVENTORY_MAIN) + 2];*/
    /*item = itemCreate();*/
    /*grass_item_block = grassItemBlockCreate();*/
    /*DYN_PTR(item, GrassItemBlock, IItem, grass_item_block);*/
    /*VCALL(*item, init);*/
    /*grass_item_block->item_block.item.stack_size = 13;*/
    /*inventorySlotSetItem(slot, item);*/
    /*VCALL_SUPER(*item, Renderable, applyInventoryRenderAttributes);*/

    Slot* slot = inventoryFindFreeSlot(inventory, 1);
    IItem* item = itemGetConstructor(ITEMID_CRAFTING_TABLE)(0);
    ItemBlock* item_block = VCAST_PTR(ItemBlock*, item);
    item_block->item.stack_size = 26;
    inventorySlotSetItem(slot, item);
    VCALL_SUPER(*item, Renderable, applyInventoryRenderAttributes);

    item = itemGetConstructor(ITEMID_COBBLESTONE)(0);
    item_block = VCAST_PTR(ItemBlock*, item);
    item_block->item.stack_size = 10;
    slot = inventoryFindFreeSlot(inventory, slotGroupIndexOffset(INVENTORY_MAIN));
    inventorySlotSetItem(slot, item);
    VCALL_SUPER(*item, Renderable, applyInventoryRenderAttributes);
    DEBUG_LOG("[Minecraft] Finished init\n");
}

void minecraftCleanup(VSelf) ALIAS("Minecraft_cleanup");
void Minecraft_cleanup(UNUSED VSelf) {
    durationTreesDestroy();
    worldDestroy(world);
    free(world);
    playerDestroy(player);
    free(player);
    assetsFree();
}

/*void handleConsole() {*/
/*    if (console == NULL) {*/
/*        console = (Console*) malloc(sizeof(Console));*/
/*    }*/
/*    while (true) {*/
/*        putchar('>');*/
/*        putchar(' ');*/
/*        memset(console->command_buffer, '\0', CONSOLE_BUFFER_SIZE);*/
/*        gets(console->command_buffer);*/
/*        if (strncmp(console->command_buffer, "exit", 4) == 0) {*/
/*            break;*/
/*        }*/
/*        consoleExecv(console);*/
/*    }*/
/*    free(console);*/
/*    console = NULL;*/
/*}*/

void minecraftInput(VSelf, const Stats* stats) ALIAS("Minecraft_input");
void Minecraft_input(VSelf, UNUSED const Stats* stats) {
    VSELF(Minecraft);
    /*if (ioctl(0, FIOCSCAN, 0) && getchar() == '`') {*/
    /*    handleConsole();*/
    /*}*/
    self->camera.mode = CAMERA_MODE_FIRST_PERSON;
    inputUpdate(&input);
}

void minecraftUpdate(VSelf, const Stats* stats) ALIAS("Minecraft_update");
void Minecraft_update(VSelf, UNUSED const Stats* stats) {
    VSELF(Minecraft);
    worldUpdate(
        world,
        player,
        &player->breaking,
        &self->ctx
    );
    playerUpdate(player, world);
    cameraUpdate(&self->camera);
}

void frustumRenderNormals(const Frustum* frustum, RenderContext* ctx) {
    // Object and light matrix for object
    MATRIX omtx, olmtx;
    // Set object rotation and position
    RotMatrix((SVECTOR*) &VEC3_I16_ZERO, &omtx);
    TransMatrix(&omtx, (VECTOR*) &VEC3_I32_ZERO);
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
                plane->normal.vx * 2,
                plane->normal.vy * 2,
                plane->normal.vz * 2
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

DEFN_DURATION_COMPONENT(render);

void minecraftRender(VSelf, const Stats* stats) ALIAS("Minecraft_render");
void Minecraft_render(VSelf, const Stats* stats) {
    VSELF(Minecraft);
#if isDebugTagEnabled(OVERLAY_DURATION_TREE)
    durationTreeMakeCurrent(&render_duration_tree);
    if (!render_duration.init) {
        render_duration = durationTreeAddComponent("render");
    } else {
        // TODO: Render duration tree here
    }
#endif
    durationComponentStart(&render_duration);
    // Update breaking state textures
    breakingStateUpdateRenderTarget(&player->breaking, &self->ctx);
    // Draw the world
    frustumTransform(&self->ctx.camera->frustum, &self->transforms);
    worldRender(
        world,
        player,
        &self->ctx,
        &self->transforms
    );
    // frustumRenderNormals(&self->ctx.camera->frustum, &self->ctx);
    frustumRestore(&self->ctx.camera->frustum);
    if (world->weather.raining || world->weather.storming) {
        weatherRender(
            world,
            player,
            &self->ctx,
            &self->transforms
        );
    }
    // Clear window constraints
    renderClearConstraints(&self->ctx);
    // Render UI
    playerRender(player, &self->ctx, &self->transforms);
    // Block UI overlays
    if (block_render_ui_context.function != NULL) {
        block_render_ui_context.function(
            &self->ctx,
            &self->transforms
        );
    }
    /*crosshairDraw(&self->ctx);*/
    axisDraw(
        &self->ctx,
        &self->transforms
    );
    renderClearConstraints(&self->ctx);
    FntPrint(0, PSXMC_VERSION_STRING "\n");
    drawDebugText(stats, &self->camera, world);
#if isDebugEnabled()
    debugDrawPacketBufferUsageGraph(
        &self->ctx,
        0,
        SCREEN_YRES - HOTBAR_HEIGHT - 20
    );
#endif
    // Flush font to screen
    FntFlush(0);
    // Swap buffers and draw the primitives
    swapBuffers(&self->ctx);
    durationComponentEnd();
}
