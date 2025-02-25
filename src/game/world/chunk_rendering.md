# Chunk Rendering

This describes the method used to traverse the loaded chunks within the world, and render only chunks that contribute to the visible subset.

## Naive Frustum Culling

An initial thought might be to just define a frustum based on the two angles that correspond to the visible space on the Y and X axis about the pitch and yaw of the camera.

Using this frustum, we can transform it to the relevant world position (this is contrary to normal in which we transform the world objects into camera space), then we loop
through all chunks and perform the following:

1. Construct the AABB of the entire 8x8x8 chunk (this can be done statically for every chunk).
2. Compute the intersection of the chunk AABB with the frustum
3. If they don't intersect, skip rendering the chunk
4. Otherwise invoke the chunk rendering logic

This issue with this is that it doesn't scale well for any moderate quanities of chunks and it doesn't take into account visibility between chunks which leads to  alot
of overdraw and visibility testing of chunks that necessarily are not visible by previous tests.

## Advanced Cave Culling

People smarter than I have already come up with better approaches to this culling problem. Namely Tommaso Checchi's [Advanced Cave Culling](https://tomcc.github.io/2014/08/31/visibility-1.html) (ACC) algorithm, which was employed
in the development of MCPE 0.9.

Essentially, it boils down to answering a single question:

> Coming from a direciton $\vec{d}$ and entering a chunk $\mathbf{C}$
> via face $\mathbf{C}_a$, is it possible to exit the chunk through
> face $\mathbf{C}_b$?

As Tommo notes in the blog, this question can be answered relatively quickly and requires only 15 bits to encode the visibility information required to answer it. Not
too dissimilar to the approach for lighting already used in PSXMC, but on a smaller scale. Note that the only time this visibility information needs to be updated is
when an opaque block is changed/added/removed in the chunk.

A slightly more formal way of approaching this is:

> Given a finite cubic mesh divided in unit intervals on all 3 axes, where each cell
> can be either filled or unfilled, let there be some subset of the cells filled. Let
> a face of the mesh be the "outer walls" where every coordinate is $0$ or $\verb{cell_count}$.
> Construct an algorithm to determine the visibility of every face from every other
> face in the mesh.

### Chunk Connectivity Graph


## Notes

For testing connectivity between chunk outer faces, if we are computing the visibility of diagonally connection faces (normal to each other), then we only need to test if any block is absent along the joining edge to garuantee the two faces are connected. This is a sufficient condition. However, if the entire edge is filled, then we must comput the connectivity the hard way.
