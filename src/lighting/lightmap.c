#include "lightmap.h"


void lightMapSetValue(LightMap lightmap,
                      const VECTOR position,
                      u8 light_value,
                      const LightType light_type) {
    const u32 index = position.vx << 11 | position.vz << 7 | position.vy;
    if (light_type == LIGHT_TYPE_SKY) {
        lightmap[index] = (lightmap[index] & 0b1111) | (light_value << 4);
    } else {
        lightmap[index] = (lightmap[index] & 0b11110000) | (light_value & 0b1111);
    }
}

u8 lightMapGetValue(const LightMap lightmap,
                    const VECTOR position,
                    const LightType light_type) {
    const u32 index = position.vx << 11 | position.vz << 7 | position.vy;
    return light_type == LIGHT_TYPE_SKY
        ? lightmap[index] & 0b1111
        : (lightmap[index] >> 4) & 0b1111;
}
