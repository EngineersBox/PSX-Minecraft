# TODO

## Implement

* [x] Implement texture loading via LZP compressed archive traversal
* [x] Complete display/render context implementation
* [x] Create standalone cube struct and load + render methods
* [x] Greedy meshing for chunk rendering
* [x] Create loading screen to visualise loading progress. Something simple like a loading bar for now. Make it more
  fancy at a later point.
* [ ] Camera far plane cutoff with fog relative to chunk render distance
* [x] Continuous shifting world chunk array
* [ ] Frustum and depth-first search culling through chunks
* [x] Graphing lib to use for resource usage and debugging
* [x] Ray cast from camera normal to block retrieving exact block coordinates in world
* [x] Dynamic chunk loading and unloading with movement
* [x] Player entity with hit box
* [x] Basic entity-mesh interaction physics with gravity
* [x] Crosshair
* [x] Axis marker
* [ ] Check if camera not moving then ray cast once to get block targeted and draw outline around if in reach range
* [ ] Thread scheduler using hardware timers for IRQ interleaved execution
* [ ] Update queue for operations on the world
* [x] Proper engine structure with logic interface
* [x] Inheritable Block interface with concrete implementations
* [x] 2D array indexed terrain texture specification in face attributes
* [ ] Breaking overlays for a block, enabled with a marker variable and a target resolved by ray cast and a texture reference
* [ ] D-pad or analogue stick controlled cursor in UIs with item movement between slots being held by cursor
* [ ] TooManyItems-like overlay in inventory GUI
* [x] Support transparent textures (original and binary greedy meshing)

## Refactor

* [x] Support texture windowing for wrap-around in multiples of texture width/height
* [ ] Move rendering handlers in `ChunkMesh` to standalone SMD renderer file
* [x] Removed aliasing wrappers for primitive/vertex/normal arrays in `ChunkMesh` and
      refactor to direct type alias of `SMD`
* [x] Camera displayed position to be units of `BLOCK_SIZE`
* [x] Y-axis values are negative upwards should be positive
* [x] Block world coords should start at 0 from bottom of chunk and go up positively.
* [x] Move crosshair render handler to UI directory with dependent structure
* [x] Move axis render handler to UI directory with dependent structure
* [x] Create loading screen structure with update/render methods and move content in world to there
* [ ] Turn these TODO list sections into tables instead of checkmark lists 
* [x] Ray cast should check distance walked is less than radius instead of out of world check
* [ ] Support other resolutions that aren't 320x240
* [x] Use geometry matrix to centre loading screen text and loading bar instead of manual positioning
* [x] Refactor `World` and `Chunk` to support interface based blocks with dynamic dispatch handlers
* [ ] Move static values like block/item names and max stack sizes to global constants accessed by ID
* [ ] Refactor vector operations to use `_Generic` C11 macro to perform type specific operations between any kind of two vector types or constant
* [x] Move to binary greedy meshing with bitwise ops for single-pass mask creation
* [x] Migrate to chunk providers and generators created with world for specific generation
* [ ] Move old mesh generation in `Chunk` into standalone implementation in `src/game/world/chunk/meshing`

## Fix

* [x] Chunk mesh traversal to be Y up instead of Z up
* [x] Last vertex in chunk not correct
* [x] Last texture window causing font render to be weird
* [x] Figure out why odd numbered chunks render, no x-valued chunks render and Y alternate chunks don't render
* [x] Figure out why `CHUNK_SIZE` that isn't 8 doesn't work (e.g. 16), in that nothing renders (no errors logged)
* [x] Fix mesh rendering of last faces connected to neighbouring chunk, normal seems inverted. 
      This might be the inverse of changes made with windowing and indexing to fix chunk rendering initially.
* [x] Camera rotation values do not wrap when a complete rotation is achieved
* [x] Fixed stale references to mesh attribute iterators (primitive/vertex/normal)
* [x] Logs spammed with GPU bad address read when looking down after a certain point
* [x] Chunk mesh construction replicates world bottom at top of world 
* [x] Chunk meshing generates redundant faces at sides of chunk
* [ ] Ray cast direction does not fully match camera rotation normal
* [ ] Mesh vertices z-depth is inconsistent leading to faces drawn in wrong order and thus culling fails
* [x] Background in loading screen not rendering texture
* [ ] Vertices are distorted (in their location) when very close to the camera
* [ ] Transparent textures rendering multiple layers of textures from different points in terrain texture
* [ ] Movement tied to FPS
* [x] Cannot fall off a block edge, get pushed back
* [x] Jumping onto a block pushes player back, not allowing for movement up to a block (weird edge cases for certain angles still work)
* [ ] Certain faces at chunk boundaries are not generated with binary greedy meshing
* [ ] Move mesh generation to after all loading when updating world to avoid face generation on orthogonal axis
* [ ] Cull faces on chunk edges that face outward on the render limit
* [ ] Chunks with a negative component in position don't generate meshes correctly (incorrect faces and textures)
