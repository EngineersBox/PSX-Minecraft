#pragma once

#ifndef CLIP_H
#define CLIP_H

#include <psxgte.h>
#include <psxgpu.h>

/* tri_clip
 *
 * Returns non-zero if a triangle (v0, v1, v2) is outside 'clip'.
 *
 * clip			- Clipping area
 * v0,v1,v2		- Triangle coordinates
 *
 */
int triClip(const RECT* clip, const DVECTOR* v0, const DVECTOR* v1, const DVECTOR *v2);

/* quad_clip
 *
 * Returns non-zero if a quad (v0, v1, v2, v3) is outside 'clip'.
 *
 * clip			- Clipping area
 * v0,v1,v2,v3	- Quad coordinates
 *
 */
int quadClip(const RECT* clip, const DVECTOR* v0, const DVECTOR* v1, const DVECTOR* v2, const DVECTOR* v3);

#endif //CLIP_H
