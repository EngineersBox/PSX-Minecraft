#include "lightmap.h"

#include "../logging/logging.h"
#include "../math/math_utils.h"
#include "../util/preprocessor.h"

const u16 LIGHT_LEVEL_COLOUR_SCALARS[16] = {
    [0]=144,
    [1]=180,
    [2]=225,
    [3]=281,
    [4]=351,
    [5]=439,
    [6]=549,
    [7]=687,
    [8]=858,
    [9]=1073,
    [10]=1342,
    [11]=1677,
    [12]=2097,
    [13]=2621,
    [14]=3276,
    [15]=4096
};

void lightMapSetValue(LightMap lightmap,
                      const VECTOR position,
                      const LightLevel light_value,
                      const LightType light_type) {
    const u32 index = position.vx << 6 | position.vz << 3 | position.vy;
    if (light_type == LIGHT_TYPE_SKY) {
        lightmap[index] = (lightmap[index] & LIGHT_BLOCK_MASK) | (light_value & LIGHT_SKY_MASK);
    } else {
        lightmap[index] = (lightmap[index] & LIGHT_SKY_MASK) | (light_value << 4);
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
        ? lightmap[index] & LIGHT_SKY_MASK
        : (lightmap[index] & LIGHT_BLOCK_MASK) >> 4;
}

INLINE LightLevel lightLevelApplicable(const LightLevel internal_light_level, const LightLevel light_value) {
    const LightLevel block = (light_value & LIGHT_BLOCK_MASK) >> 4;
    const LightLevel sky = light_value & LIGHT_SKY_MASK;
    const LightLevel internal_sky = internal_light_level & LIGHT_SKY_MASK;
    return max(block, min(sky, internal_sky));
}

INLINE u16 lightLevelColourScalar(const LightLevel internal_light_level, const LightLevel light_value) {
    return LIGHT_LEVEL_COLOUR_SCALARS[lightLevelApplicable(
        internal_light_level,
        light_value
    )];
}
