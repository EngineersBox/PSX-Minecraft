# Frustum Culling

TODO: Explain frustum creation and how queries work with dot product

* Pre-calculate frustum planes for a given Y-FOV, aspect ratio and near/far distances
* At each frame transform the frustum planes using the mirrored transform matrix (normal
  transform matrix has translation and rotation inverted since it applies to world objects,
  but we are applying to camera reference so no inversion is performed in its construction).
* Cache transformed matrix to reuse if camera doesn't move
* Compute intersection with AABB (chunk, sub-section of chunk, block, etc) and frustum planes.
  We can do the clever stuff here to do the following:
  1. Total chunk visibility check
  2. Chunk mesh direction specific culling. Mesh planes are organised into arrays for each
     direction and each array is sorted, so we can cull entire arrays based on a single normal
     and divide arrays based on frustum plane intersection with chunk to cull sub-arrays.
* Restore frustum to pre-transformed state

## Computing Rotation + Translation

The GPU is 16-bit so all ops are over SVECTORs when we have worldspace with is
typically 32-bit since the world is large. This makes the world limited to:

```
-2^15 <= CHUNK_SIZE * BLOCK_SIZE * AXIS_INDEX <= 2^15 - 1
```

Which limits the AXIS index to a range of `[-58,58]` since `8 * 70 * 58 = 32,480`
but `8 * 70 * 59 = 33,040` (similarly for negatives), so we can only have up to
58 chunks in each direction. Which equates to 464 blocks in each direction.
So instead of this we compute the matrix product and vector summation on the CPU
which is realistically just as fast and get much larger world spaces as a result.

For reference the GPU based code looks like this:
```c
SVECTOR pos_before_1 = vec3_i16(
    (chunk->position.vx * CHUNK_BLOCK_SIZE),
    (-chunk->position.vy * CHUNK_BLOCK_SIZE),
    (chunk->position.vz * CHUNK_BLOCK_SIZE)
);
SVECTOR pos_after_1 = vec3_i16(0);
PushMatrix();
gte_SetRotMatrix(&transforms->geometry_mtx);
gte_SetTransMatrix(&transforms->geometry_mtx);
gte_ldsv(&pos_before_1); // Load SVECTOR var into sv general purpose short vector register
gte_rtirtr(); // (Rotation Matrix * sv) + Translation Vector
gte_stsv(&pos_after_1); // Store sv into SVECTOR output var
PopMatrix();
```

And the comparative CPU code is:
```c
VECTOR geomMul(const MATRIX m, const VECTOR v) {
    return vec3_i32(
        (fixedMul((i32) m.m[0][0], v.vx)
            + fixedMul((i32) m.m[0][1], v.vy)
            + fixedMul((i32) m.m[0][2], v.vz)
        ) + ((i32) m.t[0] << 12),
        (fixedMul((i32) m.m[1][0], v.vx)
            + fixedMul((i32) m.m[1][1], v.vy)
            + fixedMul((i32) m.m[1][2], v.vz)
        ) + ((i32) m.t[1] << 12),
        (fixedMul((i32) m.m[2][0], v.vx)
            + fixedMul((i32) m.m[2][1], v.vy)
            + fixedMul((i32) m.m[2][2], v.vz)
        ) + ((i32) m.t[2] << 12)
    );
}
```
