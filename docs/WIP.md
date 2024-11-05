# Work In Progress

Currently looking at frustum culling and sub-chunk intersection culling in ordered meshing
interval trees.

## Frustum Culling

![Furstum Culling](./frustum_culling_psx.mov)

## Sub-Chunk Intersection Culling

Compute intersection point of frustum planes into visible chunks (if on frustum boundaries).
Determine which directions (1 - 3 faces visible max), and partition the polygon arrays for
each of those directions based on their position relative to the plane intersection as an
interval tree lookup. Resulting partitionals internal to the frustum are drawn and everything
else is excluded without concern.
