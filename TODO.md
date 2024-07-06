# TODO

## Fix

* [X]  Chunk mesh traversal to be Y up instead of Z up
* [X]  Last vertex in chunk not correct
* [X]  Last texture window causing font render to be weird
* [X]  Figure out why odd numbered chunks render, no x-valued chunks render and Y alternate chunks don't render
* [X]  Figure out why `CHUNK_SIZE` that isn't 8 doesn't work (e.g. 16), in that nothing renders (no errors logged)
* [X]  Fix mesh rendering of last faces connected to neighbouring chunk, normal seems inverted.  This might be the inverse of changes made with windowing and indexing to fix chunk rendering initially.
* [X]  Camera rotation values do not wrap when a complete rotation is achieved
* [X]  Fixed stale references to mesh attribute iterators (primitive/vertex/normal)
* [X]  Logs spammed with GPU bad address read when looking down after a certain point
* [X]  Chunk mesh construction replicates world bottom at top of world
* [X]  Chunk meshing generates redundant faces at sides of chunk
* [X]  Ray cast direction does not fully match camera rotation normal
* [X]  Background in loading screen not rendering texture
* [X]  Cannot fall off a block edge, get pushed back
* [X]  Jumping onto a block pushes player back, not allowing for movement up to a block (weird edge cases for certain angles still work)
* [X]  Certain faces at chunk boundaries are not generated with binary greedy meshing
* [X]  Chunks with a negative component in position don't generate meshes correctly (incorrect faces and textures)
* [X]  Texture attributes on block item rendering in world and picked up items in inventory
* [X]  Resource generation script does not contain new attributes for `ItemBlock` instances
* [ ]  Mesh vertices z-depth is inconsistent leading to faces drawn in wrong order and thus culling fails
* [ ]  Vertices are distorted (in their location) when very close to the camera
* [ ]  Textures in terrain tpage with any `u` and `v >= 16` rendering multiple interleaved textures from different points in terrain texture page
* [ ]  Movement tied to FPS
* [ ]  Move mesh generation to after all loading when updating world to avoid face generation on orthogonal axis to update axis
* [ ]  Cull faces on chunk edges that face outward on the render limit
* [ ]  Core engine ticks can go higher than 20, redo the engine cycle system

## Implement

* [X]  Implement texture loading via LZP compressed archive traversal
* [X]  Complete display/render context implementation
* [X]  Create standalone cube struct and load + render methods
* [X]  Greedy meshing for chunk rendering
* [X]  Create loading screen to visualise loading progress. Something simple like a loading bar for now. Make it more fancy at a later point.
* [X]  Continuous shifting world chunk array
* [X]  Graphing lib to use for resource usage and debugging
* [X]  Ray cast from camera normal to block retrieving exact block coordinates in world
* [X]  Dynamic chunk loading and unloading with movement
* [X]  Player entity with hit box
* [X]  Basic entity-mesh interaction physics with gravity
* [X]  Crosshair
* [X]  Axis marker
* [X]  Proper engine structure with logic interface
* [X]  Inheritable Block interface with concrete implementations
* [X]  2D array indexed terrain texture specification in face attributes
* [X]  Support transparent textures (original and binary greedy meshing)
* [X]  Full ASCII, extended characters and symbols set front + printing lib
* [X]  Shadowed version of font to use via flag for printing
* [X]  Breaking overlays for a block, enabled with a marker variable and a target resolved by ray cast and a texture reference
* [X]  Finish player attack input handler
* [X]  Add handling for player use handler with logic for using items and applying damage on items like fishing rods
* [X]  Hotbar active slot input handlers (left/right)
* [X]  Set block orientation relative to face placed against
* [X]  Lightmapping with flood fill algorithm using LUTs for block light (adding and removing)
* [ ]  Camera far plane cutoff with fog relative to chunk render distance
* [ ]  Frustum and culling
* [ ]  Depth-first search culling through chunks
* [ ]  Check if camera not moving then ray cast once to get block targeted and draw outline around if in reach range
* [ ]  Thread scheduler using hardware timers for IRQ interleaved execution
* [ ]  Update queue for operations on the world
* [ ]  D-pad or analogue stick controlled cursor in UIs with item movement between slots being held by cursor
* [ ]  TooManyItems-like overlay in inventory GUI
* [ ]  Retrieve face attributes based on block orientation during meshing
* [ ]  Sub-block intersection tests for raycast to handling interacting with blocks like doors and piston heads
* [ ]  If we are breaking a block on the boundary of chunks, we should pass the breaking context to both the target and neighbouring chunk in order for the mesh generation to account for the missing faces on the chunk boundary.
* [ ] Sunlight updates for chunks
* [ ] Procedural texturing for each quad in chunk mesh by rendering texture and light maps to off screen TPage and then using that to render to world.

## Refactor

* [X]  Support texture windowing for wrap-around in multiples of texture width/height
* [X]  Removed aliasing wrappers for primitive/vertex/normal arrays in `ChunkMesh` and refactor to direct type alias of `SMD`
* [X]  Camera displayed position to be units of `BLOCK_SIZE`
* [X]  Y-axis values are negative upwards should be positive
* [X]  Block world coords should start at 0 from bottom of chunk and go up positively.
* [X]  Move crosshair render handler to UI directory with dependent structure
* [X]  Move axis render handler to UI directory with dependent structure
* [X]  Create loading screen structure with update/render methods and move content in world to there
* [X]  Ray cast should check distance walked is less than radius instead of out of world check
* [X]  Use geometry matrix to centre loading screen text and loading bar instead of manual positioning
* [X]  Refactor `World` and `Chunk` to support interface based blocks with dynamic dispatch handlers
* [X]  Move to binary greedy meshing with bitwise ops for single-pass mask creation
* [X]  Migrate to chunk providers and generators created with world for specific generation
* [X]  Move old mesh generation in `Chunk` into standalone implementation in `src/game/world/chunk/meshing` or remove
* [X]  Per-face opacity control on each block for minimal meshing with mixed opacity (i.e. like farmland or stairs)
* [X]  Breaking overlay should mark target block as being broken, have a global to mark the block being broken and the breaking texture index to use, then trigger a chunk mesh regeneration on starting to break the block and when stopped (not finished breaking). Render the overlay on top of the target texture to a part of the framebuffer with `(16*6)x16` reserved to merge the texture and overlay together. Then ensure the block render tpage is poiting to the reserved area by `face_dir * 16` for that face (in the mesh data).
* [X]  Move static values like block/item names and max stack sizes to global constants accessed by ID
* [X]  Merge both font textures (with and without shadow) into a single texture to use space more efficiently and avoid the gap between the current texture file layout in VRAM.
* [X] Use custom mesh format for chunk meshes to avoid excessive overhead of unused fields with SMD structured
* [ ]  Move rendering handlers in `ChunkMesh` to standalone SMD renderer file
* [ ]  Turn these TODO list sections into tables instead of checkmark lists
* [ ]  Support other resolutions that aren't 320x240
* [ ]  Refactor vector operations to use `_Generic` C11 macro to perform type specific operations between any kind of two vector types or constant
* [ ]  Move assets to on-disk directories and files instead of packing them into the binary
