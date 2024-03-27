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
#include "../ui/ui.h"
#include "../entity/player.h"

// Reference texture data
extern const uint32_t tim_texture[];
RayCastResult result = {};

// Light color matrix
// Each column represents the color matrix of each light source and is
// used as material color when using gte_ncs() or multiplied by a
// source color when using gte_nccs(). 4096 is 1.0 in this matrix
// A column of zeroes effectively disables the light source.
MATRIX color_mtx = {
    .m = {
        {ONE * 3 / 4, 0, 0}, /* Red   */
        {ONE * 3 / 4, 0, 0}, /* Green */
        {ONE * 3 / 4, 0, 0} /* Blue  */
    }
};

// Light matrix
// Each row represents a vector direction of each light source.
// An entire row of zeroes effectively disables the light source.
MATRIX light_mtx = {
    /* X,  Y,  Z */
    .m = {
        {-FIXED_1_2, -FIXED_1_2, FIXED_1_2},
        {0, 0, 0},
        {0, 0, 0}
    }
};

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
Player* player;

void minecraftInit(VSelf, void* ctx) __attribute__((alias("Minecraft_init")));
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
            .translation_rotation = {0},
            .translation_position = {0, 0, 0},
            .geometry_mtx = {},
            .lighting_mtx = light_mtx
        },
        .input = (Input) {},
        .camera = (Camera) {}
    };
    self->internals.ctx.camera = &self->internals.camera;
    // Set light ambient color and light color matrix
    gte_SetBackColor(128, 128, 128);
    gte_SetFarColor(1, 1, 1);
    gte_SetColorMatrix(&color_mtx);
    // FOV?
    gte_SetGeomScreen(100);
    initRenderContext(&self->internals.ctx);
    initInput(&self->internals.input);
    // Load font and open a text stream
    FntLoad(960, 0);
    FntOpen(0, 8, 320, 216, 0, 150);
    // Unpack LZP archive and load assets
    assetsLoad();
    // Initialise world
    self->world = (World*) malloc(sizeof(World));
    self->world->head.vx = 0;
    self->world->head.vz = 0;
    self->world->centre.vx = 0;
    self->world->centre.vy = 0;
    self->world->centre.vz = 0;
    worldInit(self->world, &self->internals.ctx);
    world = self->world;
    cvector_init(markers, 1, NULL);
    // Initialise player
    player = (Player*) malloc(sizeof(Player));
    playerInit(player);
    player->camera = &self->internals.camera;
    player->position = self->internals.camera.position;
    player->position.vy += BLOCK_SIZE << FIXED_POINT_SHIFT;
    // ==== TESTING: Hotbar ====
    Hotbar* hotbar = VCAST(Hotbar*, player->hotbar);
    Slot* slot = &hotbar->slots[1];
    IItem* item = itemCreate();
    GrassItemBlock* grass_item_block = grassItemBlockCreate();
    DYN_PTR(item, GrassItemBlock, IItem, grass_item_block);
    VCALL(*item, init);
    grass_item_block->item_block.item.stack_size = 26;
    slot->data.item = item;
    VCALL_SUPER(*item, Renderable, applyInventoryRenderAttributes);
    // ==== TESTING: Inventory ====
    Inventory* inventory = VCAST(Inventory*, player->inventory);
    slot = &inventory->slots[INVENTORY_SLOT_STORAGE_OFFSET + 2];
    item = itemCreate();
    grass_item_block = grassItemBlockCreate();
    DYN_PTR(item, GrassItemBlock, IItem, grass_item_block);
    VCALL(*item, init);
    grass_item_block->item_block.item.stack_size = 13;
    slot->data.item = item;
    VCALL_SUPER(*item, Renderable, applyInventoryRenderAttributes);
}

void minecraftCleanup(VSelf) __attribute__((alias("Minecraft_cleanup")));
void Minecraft_cleanup(VSelf) {
    VSELF(Minecraft);
    worldDestroy(self->world);
    free(self->world);
    playerDestroy(player);
    free(player);
    assetsFree();
}

void startHandler(Camera* camera);

void minecraftInput(VSelf, const Stats* stats) __attribute__((alias("Minecraft_input")));
void Minecraft_input(VSelf, const Stats* stats) {
    VSELF(Minecraft);
    self->internals.camera.mode = 0;
    cameraUpdate(
        &self->internals.camera,
        &self->internals.input,
        &self->internals.transforms,
        &result.pos
    );
    player->position = self->internals.camera.position;
    // Player is 2 blocks high, with position caluclated at the feet
    player->position.vy += BLOCK_SIZE << FIXED_POINT_SHIFT;
    Input* input = &self->internals.input;
    if (isPressed(PAD_START)) {
        startHandler(&self->internals.camera);
    }
    if (isPressed(PAD_L1)) {
        Inventory* inventory = VCAST(Inventory*, player->inventory);
        if (!inventory->ui.active) {
            VCALL(player->inventory, open);
        } else {
            VCALL(player->inventory, close);
        }
    }
}

void minecraftUpdate(VSelf, const Stats* stats) __attribute__((alias("Minecraft_update")));
void Minecraft_update(VSelf, const Stats* stats) {
    VSELF(Minecraft);
    worldUpdate(self->world, player);
    gte_SetRotMatrix(&self->internals.transforms.geometry_mtx);
    gte_SetTransMatrix(&self->internals.transforms.geometry_mtx);
}

void startHandler(Camera* camera) {
    cvector_clear(markers);
    result = worldRayCastIntersection(world, camera, 6 * ONE, &markers);
    printf("Marker count: %d\n", cvector_size(markers));
    result.pos.vx = (result.pos.vx / BLOCK_SIZE) >> FIXED_POINT_SHIFT;
    result.pos.vz = (result.pos.vz / BLOCK_SIZE) >> FIXED_POINT_SHIFT;
    result.pos.vy = (result.pos.vy / BLOCK_SIZE) >> FIXED_POINT_SHIFT;
    printf(
        "Ray cast result: [Pos: (%d,%d,%d)] [Block: %d] [Face: (%d,%d,%d)]\n",
        inlineVec(result.pos),
        result.block == NULL ? -1 : VCAST(Block*, *result.block)->id,
        inlineVec(result.face)
    );
    camera_pos = (SVECTOR) {
        .vx = camera->position.vx >> FIXED_POINT_SHIFT,
        .vy = camera->position.vy >> FIXED_POINT_SHIFT,
        .vz = camera->position.vz >> FIXED_POINT_SHIFT,
    };
    origin_pos.vx = (((camera->position.vx / BLOCK_SIZE) >> FIXED_POINT_SHIFT) * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
    origin_pos.vy = (((camera->position.vy / BLOCK_SIZE) >> FIXED_POINT_SHIFT) * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
    origin_pos.vz = (((camera->position.vz / BLOCK_SIZE) >> FIXED_POINT_SHIFT) * BLOCK_SIZE) + (BLOCK_SIZE >> 1);
    marker_pos.vx =  (result.pos.vx * BLOCK_SIZE) + (BLOCK_SIZE >> 1); // + ((result.face.vx >> FIXED_POINT_SHIFT) * (BLOCK_SIZE >> 1));
    marker_pos.vy = (-result.pos.vy * BLOCK_SIZE) - (BLOCK_SIZE >> 1); // + ((result.face.vy >> FIXED_POINT_SHIFT) * (BLOCK_SIZE >> 1));
    marker_pos.vz =  (result.pos.vz * BLOCK_SIZE) + (BLOCK_SIZE >> 1); // + ((result.face.vz >> FIXED_POINT_SHIFT) * (BLOCK_SIZE >> 1));
    // TODO: When removing a block (as it takes time to break) should the camera be locked in
    //       place as to avoid needing to continually raycast each frame until the block is broken
    //       or the trigger/key/mouse is unpressed?
    IItem* item = NULL;
    worldModifyVoxel(world, &result.pos, airBlockCreate(), &item);
    printf("Origin: (%d,%d,%d)\n", inlineVec(origin_pos));
    printf(
        "Marker: (%d,%d,%d) Camera: (%d,%d,%d)\n",
        inlineVec(marker_pos),
        inlineVec(camera->position)
    );
    const VECTOR direction = rotationToDirection(&camera->rotation);
    printf("Direction: (%d,%d,%d)\n", inlineVec(direction));
    direction_pos = (SVECTOR) {
        .vx = (camera->position.vx + (direction.vx * BLOCK_SIZE)) >> FIXED_POINT_SHIFT,
        .vy = (camera->position.vy - (direction.vy * BLOCK_SIZE)) >> FIXED_POINT_SHIFT,
        .vz = (camera->position.vz + (direction.vz * BLOCK_SIZE)) >> FIXED_POINT_SHIFT
    };
    printf(
        "CPOS: (%d,%d,%d) DPOS: (%d,%d,%d)\n",
        inlineVec(origin_pos),
        inlineVec(direction_pos)
    );
    render_marker = true;
    SVECTOR* cmarker;
    cvector_for_each_in(cmarker, markers) {
        printf("[TRACE] MARKER: (%d,%d,%d)\n", inlineVecPtr(cmarker));
    }
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
//     // printf("CURRENT MARKER: (%d,%d,%d)\n", inlineVecPtr(current_marker));
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
    const Camera* camera = &minecraft->internals.camera;
    const int32_t x = camera->position.vx / BLOCK_SIZE;
    const int32_t y_down = camera->position.vy / BLOCK_SIZE;
    const int32_t y_up = -camera->position.vy / BLOCK_SIZE;
    const int32_t z = camera->position.vz / BLOCK_SIZE;
    FntPrint(
        0,
        ""
        "X=%d.%05d\n"
        "Y=%d.%05d (DOWN)\n"
        "Y=%d.%05d (UP)\n"
        "Z=%d.%05d\n",
        fixedGetWhole(x), fixedGetFractional(x),
        fixedGetWhole(y_down), fixedGetFractional(y_down),
        fixedGetWhole(y_up), fixedGetFractional(y_up),
        fixedGetWhole(z), fixedGetFractional(z)
    );
    FntPrint(
        0,
        "RX=%d RY=%d\n",
        camera->rotation.vx >> FIXED_POINT_SHIFT,
        camera->rotation.vy >> FIXED_POINT_SHIFT
    );
    const VECTOR direction = rotationToDirection(&camera->rotation);
    FntPrint(
        0,
        "DX=%d DY=%d DZ=%d\n",
        inlineVec(direction)
    );
}

void minecraftRender(VSelf, const Stats* stats) __attribute__((alias("Minecraft_render")));
void Minecraft_render(VSelf, const Stats* stats) {
    VSELF(Minecraft);
    // Draw the world
    worldRender(
        self->world,
        &self->internals.ctx,
        &self->internals.transforms
    );
    // Clear window constraints
    renderClearConstraints(&self->internals.ctx);
    // Draw marker
    drawMarker(self);
    // Render UI
    playerRender(player, &self->internals.ctx, &self->internals.transforms);
    // crosshairDraw(&render_context);
    drawDebugText(self, stats);
    axisDraw(&self->internals.ctx, &self->internals.transforms, &self->internals.camera);
    debugDrawPBUsageGraph(&self->internals.ctx, 0, SCREEN_YRES);
    // Flush font to screen
    FntFlush(0);
    // Swap buffers and draw the primitives
    swapBuffers(&self->internals.ctx);
}