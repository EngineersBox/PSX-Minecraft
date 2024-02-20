/*
 * Polygon clip detection code
 *
 * The polygon clipping logic is based on the Cohen-Sutherland algorithm, but
 * only the off-screen detection logic is used to determine which polygon edges
 * are off-screen.
 *
 * In tri_clip, the following edges are checked as follows:
 *
 *  0
 *  |\
 *  |  \
 *  |    \
 *  |      \
 *  |--------
 *  1       2
 *
 * In quad_clip, the following edges are checked as follows:
 *
 *  0         1
 *  |---------|
 *  | \     / |
 *  |   \ /   |
 *  |   / \   |
 *  | /     \ |
 *  |---------|
 *  2         3
 *
 * The inner portion of the quad is checked, otherwise the quad will be
 * culled out if the camera faces right into it, where all four edges
 * are off-screen at once.
 *
 */

#include "clip.h"

#define CLIP_LEFT 1
#define CLIP_RIGHT 2
#define CLIP_TOP 4
#define CLIP_BOTTOM	8

int testClip(const RECT* clip,
			  const int16_t x,
			  const int16_t y) {
	// Tests which corners of the screen a point lies outside of
	int result = 0;
	if (x < clip->x) {
		result |= CLIP_LEFT;
	}
	if (x >= clip->x + (clip->w - 1)) {
		result |= CLIP_RIGHT;
	}
	if (y < clip->y) {
		result |= CLIP_TOP;
	}
	if (y >= clip->y + (clip->h - 1)) {
		result |= CLIP_BOTTOM;
	}
	return result;
}

int triClip(const RECT* clip,
			 const DVECTOR* v0,
			 const DVECTOR* v1,
			 const DVECTOR* v2) {
	// Returns non-zero if a triangle is outside the screen boundaries
	int16_t c[3];
	c[0] = testClip(clip, v0->vx, v0->vy);
	c[1] = testClip(clip, v1->vx, v1->vy);
	c[2] = testClip(clip, v2->vx, v2->vy);
	return (c[0] & c[1])
		&& (c[1] & c[2])
		&& (c[2] & c[0]);
}

int quadClip(const RECT* clip,
			  const DVECTOR* v0,
			  const DVECTOR* v1,
			  const DVECTOR* v2,
			  const DVECTOR* v3) {
	// Returns non-zero if a quad is outside the screen boundaries
	int16_t c[4];
	c[0] = testClip(clip, v0->vx, v0->vy);
	c[1] = testClip(clip, v1->vx, v1->vy);
	c[2] = testClip(clip, v2->vx, v2->vy);
	c[3] = testClip(clip, v3->vx, v3->vy);
	return (c[0] & c[1])
		&& (c[1] & c[2])
		&& (c[2] & c[3])
		&& (c[3] & c[0])
		&& (c[0] & c[2])
		&& (c[1] & c[3]);
}