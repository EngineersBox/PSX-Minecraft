#include "lightmap.h"

#include "../logging/logging.h"
#include "../math/math_utils.h"
#include "../util/preprocessor.h"

void lightMapSetValue(LightMap lightmap,
                      const VECTOR position,
                      u8 light_value,
                      const LightType light_type) {
    const u32 index = position.vx << 6 | position.vz << 3 | position.vy;
    if (light_type == LIGHT_TYPE_SKY) {
        lightmap[index] = (lightmap[index] & LIGHT_SKY_MASK) | (light_value << 4);
    } else {
        lightmap[index] = (lightmap[index] & LIGHT_BLOCK_MASK) | (light_value & LIGHT_SKY_MASK);
    }
}

u8 lightMapGetValue(const LightMap lightmap, const VECTOR position) {
    const u32 index = position.vx << 11 | position.vz << 7 | position.vy;
    return lightmap[index];
}

u8 lightMapGetType(const LightMap lightmap,
                   const VECTOR position,
                   const LightType light_type) {
    const u32 index = position.vx << 11 | position.vz << 7 | position.vy;
    return light_type == LIGHT_TYPE_SKY
        ? lightmap[index] & 0b1111
        : (lightmap[index] >> 4) & 0b1111;
}

u8 lightLevelToOverlayColour(const u8 light_value) {
    const u8 block = (light_value & LIGHT_BLOCK_MASK) >> 4;
    const u8 sky = light_value & LIGHT_SKY_MASK;
    const u8 max_light = max(block, sky);
    return (LIGHT_FIXED_AMOUNT * max_light) >> FIXED_POINT_SHIFT;
}

// (128 * 4096) >> 12
// 34,952.5333333333
