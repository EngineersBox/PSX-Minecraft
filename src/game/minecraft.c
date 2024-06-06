#include "minecraft.h"

#include <inline_c.h>
#include <interface99_extensions.h>
#include <item_block_grass.h>
#include <item_block_stone.h>
#include <psxgpu.h>
#include <psxgte.h>

#include "../ui/axis.h"
#include "../structure/primitive/clip.h"
#include "../structure/primitive/cube.h"
#include "../structure/cvector_utils.h"
#include "../render/debug.h"
#include "../math/math_utils.h"
#include "../math/vector.h"
#include "../ui/ui.h"
#include "../entity/player.h"
#include "../resources/assets.h"
#include "../render/font.h"

// Reference texture data
extern const uint32_t tim_texture[];
RayCastResult result = {};

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

bool render_marker = false;
cvector(SVECTOR) markers = NULL;
VECTOR zero_vec = {0};
SVECTOR zero_svec = {0};

SVECTOR camera_pos = {0};
SVECTOR origin_pos = {0};
SVECTOR marker_pos = {0};
SVECTOR marker_rot = {0};
SVECTOR direction_pos = {0};

World* world;
Camera camera;
IInputHandler player_handler;
Player* player;

FontID font_id;

// ONE_BLOCK * 1.7
#define CAMERA_OFFSET 487424
// #define CAMERA_OFFSET ONE_BLOCK

void minecraftInit(VSelf, void* ctx) ALIAS("Minecraft_init");
void Minecraft_init(VSelf, void* ctx) {
    VSELF(Minecraft);
    blocksInitialiseBuiltin();
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
    // Load font and open a text stream
    FntLoad(960, 0);
    FntOpen(0, 8, 320, 216, 0, 150);
    // Unpack LZP archive and load assets
    assetsLoad();
    fontLoad();
    font_id = fontOpen(0, 0, 128, 128, 0, 256 + 16);
    // Initialise world
    self->world = (World*) malloc(sizeof(World));
    self->world->head.vx = 0;
    self->world->head.vz = 0;
    self->world->centre.vx = 0;
    self->world->centre.vy = 0;
    self->world->centre.vz = 0;
    DYN_PTR(
        &self->world->chunk_provider,
        OverworldPerlinChunkProvider,
        IChunkProvider,
        malloc(sizeof(OverworldPerlinChunkProvider))
    );
    worldInit(self->world, &self->internals.ctx);
    world = self->world;
    cvector_init(markers, 1, NULL);
    // Initialise player
    player = (Player*) malloc(sizeof(Player));
    playerInit(player);
    player->camera = &self->internals.camera;
    const VECTOR player_positon = vec3_i32(0, CHUNK_BLOCK_SIZE + (BLOCK_SIZE * 2), 0);
    iPhysicsObjectSetPosition(&player->physics_object, &player_positon);
    player->physics_object.flags.no_clip = true;
    player_handler = DYN(Player, IInputHandler, player);
    VCALL(player_handler, registerInputHandler, &self->internals.input);
    // Register handlers
    VCALL(*player->camera, registerInputHandler, &self->internals.input);
    VCALL_SUPER(player->inventory, IInputHandler, registerInputHandler, &self->internals.input);
    VCALL_SUPER(player->hotbar, IInputHandler, registerInputHandler, &self->internals.input);

    // Initialise camera
    Camera* camera = VCAST(Camera*, self->internals.camera);
    camera->rotation.vx = player->physics_object.rotation.pitch;
    camera->rotation.vy = player->physics_object.rotation.yaw;
    camera->position.vx = player->physics_object.position.vx;
    camera->position.vy = -player->physics_object.position.vy - CAMERA_OFFSET;
    camera->position.vz = player->physics_object.position.vz;

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
    if (isPressed(input->pad, BINDING_ATTACK)) {
        startHandler(camera);
    }
}

void minecraftUpdate(VSelf, const Stats* stats) ALIAS("Minecraft_update");
void Minecraft_update(VSelf, const Stats* stats) {
    VSELF(Minecraft);
    worldUpdate(self->world, player);
    playerUpdate(player, world);
    Camera* camera = VCAST(Camera*, self->internals.camera);
    camera->rotation.vx = player->physics_object.rotation.pitch;
    camera->rotation.vy = player->physics_object.rotation.yaw;
    camera->position.vx = player->physics_object.position.vx;
    camera->position.vy = -player->physics_object.position.vy - CAMERA_OFFSET;
    camera->position.vz = player->physics_object.position.vz;
}

void startHandler(Camera* camera) {
    // cvector_clear(markers);
    result = worldRayCastIntersection(world, camera, 6, &markers);
    // printf("Marker count: %d\n", cvector_size(markers));
    // result.pos.vx = (result.pos.vx / BLOCK_SIZE) >> FIXED_POINT_SHIFT;
    // result.pos.vz = (result.pos.vz / BLOCK_SIZE) >> FIXED_POINT_SHIFT;
    // result.pos.vy = (result.pos.vy / BLOCK_SIZE) >> FIXED_POINT_SHIFT;
    printf(
        "Ray cast result: [Pos: " VEC_PATTERN "] [Block: %d] [Face: " VEC_PATTERN "]\n",
        VEC_LAYOUT(result.pos),
        result.block == NULL ? -1 : VCAST(Block*, *result.block)->id,
        VEC_LAYOUT(result.face)
    );
    camera_pos = (SVECTOR) {
        .vx = camera->position.vx >> FIXED_POINT_SHIFT,
        .vy = camera->position.vy >> FIXED_POINT_SHIFT,
        .vz = camera->position.vz >> FIXED_POINT_SHIFT,
    };
    // origin_pos.vx = (((camera->position.vx / BLOCK_SIZE) >> FIXED_POINT_SHIFT) * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
    // origin_pos.vy = (((camera->position.vy / BLOCK_SIZE) >> FIXED_POINT_SHIFT) * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
    // origin_pos.vz = (((camera->position.vz / BLOCK_SIZE) >> FIXED_POINT_SHIFT) * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
    // marker_pos.vx =  (result.pos.vx * BLOCK_SIZE) + (BLOCK_SIZE >> 1); // + ((result.face.vx >> FIXED_POINT_SHIFT) * (BLOCK_SIZE >> 1));
    // marker_pos.vy = (-result.pos.vy * BLOCK_SIZE) - (BLOCK_SIZE >> 1); // + ((result.face.vy >> FIXED_POINT_SHIFT) * (BLOCK_SIZE >> 1));
    // marker_pos.vz =  (result.pos.vz * BLOCK_SIZE) + (BLOCK_SIZE >> 1); // + ((result.face.vz >> FIXED_POINT_SHIFT) * (BLOCK_SIZE >> 1));
    // // TODO: When removing a block (as it takes time to break) should the camera be locked in
    // //       place as to avoid needing to continually raycast each frame until the block is broken
    // //       or the trigger/key/mouse is unpressed?
    IItem* item = NULL;
    worldModifyVoxel(world, &result.pos, airBlockCreate(), &item);
    // printf("Origin: " VEC_PATTERN "\n", VEC_LAYOUT(origin_pos));
    // printf(
    //     "Marker: " VEC_PATTERN " Camera: " VEC_PATTERN "\n",
    //     VEC_LAYOUT(marker_pos),
    //     VEC_LAYOUT(camera->position)
    // );
    // const VECTOR direction = rotationToDirection(&camera->rotation);
    // printf("Direction: " VEC_PATTERN "\n", VEC_LAYOUT(direction));
    // direction_pos = (SVECTOR) {
    //     .vx = (camera->position.vx + (direction.vx * BLOCK_SIZE)) >> FIXED_POINT_SHIFT,
    //     .vy = (camera->position.vy - (direction.vy * BLOCK_SIZE)) >> FIXED_POINT_SHIFT,
    //     .vz = (camera->position.vz + (direction.vz * BLOCK_SIZE)) >> FIXED_POINT_SHIFT
    // };
    // printf(
    //     "CPOS: " VEC_PATTERN " DPOS: " VEC_PATTERN "\n",
    //     VEC_LAYOUT(origin_pos),
    //     VEC_LAYOUT(direction_pos)
    // );
    // render_marker = true;
    // SVECTOR* cmarker;
    // cvector_for_each_in(cmarker, markers) {
    //     printf("[TRACE] MARKER: " VEC_PATTERN "\n", VEC_PTR_LAYOUT(cmarker));
    // }
}

void drawDirectionLine(Minecraft* minecraft) {
    MATRIX omtx, olmtx;
    // Ray trace line
    RotMatrix(&zero_svec, &omtx);
    TransMatrix(&omtx, &zero_vec);
    // Multiply light matrix to object matrix
    MulMatrix0(&minecraft->internals.transforms.lighting_mtx, &omtx, &olmtx);
    // Set result to GTE light matrix
    gte_SetLightMatrix(&olmtx);
    // Composite coordinate matrix transform, so object will be rotated and
    // positioned relative to camera matrix (mtx), so it'll appear as
    // world-space relative.
    CompMatrixLV(&minecraft->internals.transforms.geometry_mtx, &omtx, &omtx);
    // Save matrix
    PushMatrix();
    // Set matrices
    gte_SetRotMatrix(&omtx);
    gte_SetTransMatrix(&omtx);
    // Generate line
    LINE_F2* line = (LINE_F2*) allocatePrimitive(&minecraft->internals.ctx, sizeof(LINE_F2));
    setLineF2(line);
    gte_ldv0(&origin_pos);
    // Rotation, Translation and Perspective Single
    gte_rtps();
    gte_stsxy(&line->x0);
    gte_ldv0(&direction_pos);
    // Rotation, Translation and Perspective Single
    gte_rtps();
    gte_stsxy(&line->x1);
    setRGB0(
        line,
        0x0,
        0xff,
        0x0
    );
    uint32_t* ot_entry = allocateOrderingTable(&minecraft->internals.ctx, 0);
    addPrim(ot_entry, line);
    PopMatrix();
}

void drawRayLine(Minecraft* minecraft) {
    MATRIX omtx, olmtx;
    // Ray trace line
    RotMatrix(&zero_svec, &omtx);
    TransMatrix(&omtx, &zero_vec);
    // Multiply light matrix to object matrix
    MulMatrix0(&minecraft->internals.transforms.lighting_mtx, &omtx, &olmtx);
    // Set result to GTE light matrix
    gte_SetLightMatrix(&olmtx);
    // Composite coordinate matrix transform, so object will be rotated and
    // positioned relative to camera matrix (mtx), so it'll appear as
    // world-space relative.
    CompMatrixLV(&minecraft->internals.transforms.geometry_mtx, &omtx, &omtx);
    // Save matrix
    PushMatrix();
    // Set matrices
    gte_SetRotMatrix(&omtx);
    gte_SetTransMatrix(&omtx);
    // Generate line
    LINE_F2* line = (LINE_F2*) allocatePrimitive(&minecraft->internals.ctx, sizeof(LINE_F2));
    setLineF2(line);
    gte_ldv0(&origin_pos);
    // Rotation, Translation and Perspective Single
    gte_rtps();
    gte_stsxy(&line->x0);
    gte_ldv0(&marker_pos);
    // Rotation, Translation and Perspective Single
    gte_rtps();
    gte_stsxy(&line->x1);
    setRGB0(
        line,
        0x0,
        0x0,
        0xff
    );
    uint32_t* ot_entry = allocateOrderingTable(&minecraft->internals.ctx, 0);
    addPrim(ot_entry, line);
    PopMatrix();
}

void drawMarker(Minecraft* minecraft) {
    if (!render_marker) {
        return;
    }
    VECTOR zero = {0};
    drawDirectionLine(minecraft);
    drawRayLine(minecraft);
    // Trace end marker
    // marker_rot.vy += 16;
    // marker_rot.vz += 16;
//     POLY_F4* pol4;
//     int p;
//     MATRIX omtx, olmtx;
//     // printf("CURRENT MARKER: " VEC_PATTERN "\n", VEC_PTR_LAYOUT(current_marker));
//     // Set object rotation and position
//     RotMatrix(&marker_rot, &omtx);
//     TransMatrix(&omtx, &zero);
//     // Multiply light matrix to object matrix
//     MulMatrix0(&minecraft->internals.transforms.lighting_mtx, &omtx, &olmtx);
//     // Set result to GTE light matrix
//     gte_SetLightMatrix(&olmtx);
//     // Composite coordinate matrix transform, so object will be rotated and
//     // positioned relative to camera matrix (mtx), so it'll appear as
//     // world-space relative.
//     CompMatrixLV(&minecraft->internals.transforms.geometry_mtx, &omtx, &omtx);
//     // Save matrix
//     PushMatrix();
//     cvector_iterator(SVECTOR) current_marker;
//     cvector_for_each_in(current_marker, markers) {
//         // Set matrices
//         gte_SetRotMatrix(&omtx);
//         gte_SetTransMatrix(&omtx);
//         for (int i=0; i<8; i++) {
//             pol4 = (POLY_F4*) allocatePrimitive(&minecraft->internals.ctx, sizeof(POLY_F4));
// #define createVert(_v) (SVECTOR) { \
//             current_marker->vx + verts[CUBE_INDICES[i]._v].vx, \
//             current_marker->vy + verts[CUBE_INDICES[i]._v].vy, \
//             current_marker->vz + verts[CUBE_INDICES[i]._v].vz, \
//             0 \
//         }
//             SVECTOR current_verts[4] = {
//                 createVert(v0),
//                 createVert(v1),
//                 createVert(v2),
//                 createVert(v3)
//             };
//             gte_ldv3(
//                 &current_verts[0],
//                 &current_verts[1],
//                 &current_verts[2]
//             );
//             // Rotation, Translation and Perspective Triple
//             gte_rtpt();
//             gte_nclip();
//             gte_stopz(&p);
//             if (p < 0) {
//                 freePrimitive(&minecraft->internals.ctx, sizeof(POLY_F4));
//                 continue;
//             }
//             // Average screen Z result for four primtives
//             gte_avsz4();
//             gte_stotz(&p);
//             // Initialize a textured quad primitive
//             setPolyF4(pol4);
//             // Set the projected vertices to the primitive
//             gte_stsxy0(&pol4->x0);
//             gte_stsxy1(&pol4->x1);
//             gte_stsxy2(&pol4->x2);
//             // Compute the last vertex and set the result
//             gte_ldv0(&current_verts[3]);
//             gte_rtps();
//             gte_stsxy(&pol4->x3);
//             // Test if quad is off-screen, discard if so
//             if (quadClip(
//                 &minecraft->internals.ctx.screen_clip,
//                 (DVECTOR*) &pol4->x0,
//                 (DVECTOR*) &pol4->x1,
//                 (DVECTOR*) &pol4->x2,
//                 (DVECTOR*) &pol4->x3)) {
//                 freePrimitive(&minecraft->internals.ctx, sizeof(POLY_F4));
//                 continue;
//                 }
//             setRGB0(
//                 pol4,
//                 0xff,
//                 0x0,
//                 0x0
//             );
//             gte_ldrgb(&pol4->r0);
//             // Load the face normal
//             gte_ldv0(&CUBE_NORMS[i]);
//             // Normal Color Column Single
//             gte_nccs();
//             // Store result to the primitive
//             gte_strgb(&pol4->r0);
//             uint32_t* ot_entry = allocateOrderingTable(&minecraft->internals.ctx, 0);
//             addPrim(ot_entry, pol4);
//         }
//     }
//     PopMatrix();
}

void drawDebugText(const Minecraft* minecraft, const Stats* stats) {
    FntPrint(0, "FPS=%d TPS=%d\n", stats->fps, stats->tps);
    const int32_t x = camera.position.vx / BLOCK_SIZE;
    const int32_t y_down = camera.position.vy / BLOCK_SIZE;
    const int32_t y_up = -camera.position.vy / BLOCK_SIZE;
    const int32_t z = camera.position.vz / BLOCK_SIZE;
#define fracToFloat(frac) ((u32)(100000 * ((frac) / 4096.0)))
// #define fracToFloat(frac) (frac)
    FntPrint(
        0,
        ""
        "X=%d.%05d\n"
        "Y=%d.%05d (DOWN)\n"
        "Y=%d.%05d (UP)\n"
        "Z=%d.%05d\n",
        fixedGetWhole(x), fracToFloat(fixedGetFractional(x)),
        fixedGetWhole(y_down), fracToFloat(fixedGetFractional(y_down)),
        fixedGetWhole(y_up), fracToFloat(fixedGetFractional(y_up)),
        fixedGetWhole(z), fracToFloat(fixedGetFractional(z))
    );
    FntPrint(
        0,
        "RX=%d RY=%d\n",
        camera.rotation.vx >> FIXED_POINT_SHIFT,
        camera.rotation.vy >> FIXED_POINT_SHIFT
    );
    const VECTOR direction = rotationToDirection(&camera.rotation);
    FntPrint(
        0,
        "DX=%d DY=%d DZ=%d\n",
        VEC_LAYOUT(direction)
    );
}

void printAllFontPoints(RenderContext* ctx) {
    for (int i = 0; i < 16; i++) {
        const char c = i * 16;
        // char str[18] = {0};
        // sprintf(
        //     str,
        //     "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
        //     c, c + 1, c + 2, c + 3, c + 4, c + 5, c + 6, c + 7,
        //     c + 8, c + 9, c + 10, c + 11, c + 12, c + 13, c + 14, c + 15
        // );
        // ctx->primitive = fontSort(
        //     ctx->db[ctx->active].ordering_table,
        //     ctx->primitive,
        //     0, i * FONT_CHARACTER_SPRITE_HEIGHT,
        //     str
        // );
        fontPrint(
            font_id,
            "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
            c == 0 ? ' ' : c, c + 1, c + 2, c + 3, c + 4, c + 5, c + 6, c + 7,
            c + 8, c + 9, c + 10, c + 11, c + 12, c + 13, c + 14, c + 15
        );
    }
}

void minecraftRender(VSelf, const Stats* stats) ALIAS("Minecraft_render");
void Minecraft_render(VSelf, const Stats* stats) {
    VSELF(Minecraft);
    // Draw the world
    // frustumTransform(&self->internals.ctx.camera->frustum, &self->internals.transforms);
    // worldRender(
    //     self->world,
    //     &self->internals.ctx,
    //     &self->internals.transforms
    // );
    // frustumRestore(&self->internals.ctx.camera->frustum);
    // Clear window constraints
    renderClearConstraints(&self->internals.ctx);
    // Draw marker
    // drawMarker(self);
    // Render UI
    // playerRender(player, &self->internals.ctx, &self->internals.transforms);
    // crosshairDraw(&render_context);
    // drawDebugText(self, stats);
    // axisDraw(&self->internals.ctx, &self->internals.transforms, &camera);
    // debugDrawPBUsageGraph(
    //     &self->internals.ctx,
    //     0,
    //     SCREEN_YRES - HOTBAR_HEIGHT - 2
    // );
    // Flush font to screen
    printAllFontPoints(&self->internals.ctx);
    fontFlush(font_id);
    FntFlush(0);
    // Swap buffers and draw the primitives
    swapBuffers(&self->internals.ctx);
}