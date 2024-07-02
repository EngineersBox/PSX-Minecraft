#include "lightmap.h"

void lightMapSetBlock(LightMap lightmap, const VECTOR position, u8 light_value) {
    const u32 packed_coords_index = position.vx << 11 | position.vz << 7 | position.vy;
    const u32 direction_coord_index = packed_coords_index >> 1;
    const u32 coords_sign = packed_coords_index & 1;
    if(coords_sign == 0) {
        lightmap[direction_coord_index] = (u8)((lightmap[direction_coord_index] & 0b11110000) | (light_value & 0b1111));
    } else {
        lightmap[direction_coord_index] = (u8)((lightmap[direction_coord_index] & 0b1111) | (light_value & 0b1111) << 4);
    }
}

u8 lightMapGetBlock(const LightMap lightmap, const VECTOR position) {
    const u32 packed_coords_index = position.vx << 11 | position.vz << 7 | position.vy;
    const u32 direction_coords_index = packed_coords_index >> 1;
    const u32 coords_sign = packed_coords_index & 1;
    return coords_sign == 0 ? lightmap[direction_coords_index] & 0b1111 : lightmap[direction_coords_index] >> 4 & 0b1111;
}
