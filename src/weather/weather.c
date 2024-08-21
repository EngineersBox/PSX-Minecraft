#include "weather.h"

#include "../util/preprocessor.h"
#include "../game/world/position.h"

FWD_DECL ChunkHeightmap* worldGetChunkHeightmap(World* world, const VECTOR* position);

void renderWeatherOverlay(const World* world,
                          const Player* player,
                          RenderContext* ctx) {
    const VECTOR player_pos = vec3_i32(
        player->physics_object.position.vx >> FIXED_POINT_SHIFT,
        -player->physics_object.position.vy >> FIXED_POINT_SHIFT,
        player->physics_object.position.vz >> FIXED_POINT_SHIFT
    );
    ChunkBlockPosition cb_pos;
    for (i32 x = player_pos.vx - WEATHER_RENDER_RADIUS; x < player_pos.vx + WEATHER_RENDER_RADIUS; x++) {
        for (i32 z = player_pos.vz - WEATHER_RENDER_RADIUS; z < player_pos.vz + WEATHER_RENDER_RADIUS; z++) {
            const VECTOR pos = vec3_add(player_pos, vec3_i32(x, 0, z));
            cb_pos = worldToChunkBlockPosition(&pos, CHUNK_SIZE);
            ChunkHeightmap* heightmap = worldGetChunkHeightmap((World*) world, &cb_pos.chunk);
            const i32 top_solid_block_y = (*heightmap)[chunkHeightmapIndex(
                cb_pos.block.vx,
                cb_pos.block.vz
            )];
            const i32 y_bottom = max(player_pos.vy - WEATHER_RENDER_RADIUS, top_solid_block_y);
            const i32 y_top = max(player_pos.vy + WEATHER_RENDER_RADIUS, top_solid_block_y);
            if (y_bottom == y_top) {
                continue;
            }
            // TODO: render quads based on xz with y_bottom and y_top as limits
        }
    }
}
