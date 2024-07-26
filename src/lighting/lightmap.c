#include "lightmap.h"

#include "../logging/logging.h"
#include "../math/math_utils.h"
#include "../util/preprocessor.h"

void lightMapSetValue(LightMap lightmap,
                      const VECTOR position,
                      LightLevel light_value,
                      const LightType light_type) {
    const u32 index = position.vx << 6 | position.vz << 3 | position.vy;
    if (light_type == LIGHT_TYPE_SKY) {
        lightmap[index] = (lightmap[index] & LIGHT_SKY_MASK) | (light_value << 4);
    } else {
        lightmap[index] = (lightmap[index] & LIGHT_BLOCK_MASK) | (light_value & LIGHT_SKY_MASK);
    }
}

INLINE LightLevel lightMapGetValue(const LightMap lightmap, const VECTOR position) {
    const u32 index = position.vx << 6 | position.vz << 3 | position.vy;
    return lightmap[index];
}

INLINE LightLevel lightMapGetType(const LightMap lightmap,
                          const VECTOR position,
                          const LightType light_type) {
    const u32 index = position.vx << 6 | position.vz << 3 | position.vy;
    return light_type == LIGHT_TYPE_SKY
        ? lightmap[index] & 0b1111
        : (lightmap[index] >> 4) & 0b1111;
}

INLINE LightLevel lightLevelApplicable(const LightLevel internal_light_level, const LightLevel light_value) {
    const LightLevel block = (light_value & LIGHT_BLOCK_MASK) >> 4;
    const LightLevel sky = light_value & LIGHT_SKY_MASK;
    const LightLevel internal_sky = internal_light_level & LIGHT_SKY_MASK;
    return max(block, min(sky, internal_sky));
}

INLINE LightLevel lightLevelToOverlayColour(const LightLevel light_value) {
    // This is equivalent to doing the following:
    // BASE: (128 * ONE) / 16 = 32768
    // COLOUR: (36768 * (max_light + 1)) >> FIXED_POINT_SHIFT
    // Since 32768 == 0b1000000000000000 == 1 << 15, this means
    // multiplying it by any value is the same as shifting that
    // value over by 15. We then right shift that result by 12,
    // which is the same as if we just left shifted the original
    // multiplier by 3. Hence, (max_light + 1) << 3. Note that
    // the + 1 is just to ensure that the lowest light level is
    // not absolute black, which is the same as transparent on the
    // PS1's GPU.
    return (lightLevelApplicable(15, light_value) + 1) << 3;
}
