# TODO

## Fix

* [X] Chunk mesh traversal to be Y up instead of Z up
* [X] Last vertex in chunk not correct
* [X] Last texture window causing font render to be weird
* [X] Figure out why odd numbered chunks render, no x-valued chunks render and Y alternate chunks don't render
* [X] Figure out why `CHUNK_SIZE` that isn't 8 doesn't work (e.g. 16), in that nothing renders (no errors logged)
* [X] Fix mesh rendering of last faces connected to neighbouring chunk, normal seems inverted. This might be the inverse of changes made with windowing and indexing to fix chunk rendering initially.
* [X] Camera rotation values do not wrap when a complete rotation is achieved
* [X] Fixed stale references to mesh attribute iterators (primitive/vertex/normal)
* [X] Logs spammed with GPU bad address read when looking down after a certain point
* [X] Chunk mesh construction replicates world bottom at top of world
* [X] Chunk meshing generates redundant faces at sides of chunk
* [X] Ray cast direction does not fully match camera rotation normal
* [X] Background in loading screen not rendering texture
* [X] Cannot fall off a block edge, get pushed back
* [X] Jumping onto a block pushes player back, not allowing for movement up to a block (weird edge cases for certain angles still work)
* [X] Certain faces at chunk boundaries are not generated with binary greedy meshing
* [X] Chunks with a negative component in position don't generate meshes correctly (incorrect faces and textures)
* [X] Texture attributes on block item rendering in world and picked up items in inventory
* [X] Resource generation script does not contain new attributes for `ItemBlock` instances
* [X] Propagating light should be considered for blocks that are not solid, not just whether they are `BLOCKTYPE_EMPTY`
* [X] Item world position is incorrect causing rendering issues and physics seems to cause items to rise into the air despite no collision
* [X] Block item base position currently uses `AABB->max`, however we should use `AABB->min` and fix the render offsets + bobbing since it intersects with the block below
* [X] Dropped items don't have world lighting correctly applied to them. We should also query the lightmap to determine how lit the item should be scaled by world lighting the same way that terrain is lit.
* [X] If target block changes while holding down break, both new and old block have breaking animation but only new block actually breaks
* [X] Items don't consider new world position when checking updates, so they cannot be picked up
* [X] Core engine ticks can go higher than 20, redo the engine cycle system
* [X] Inventory opens correctly and renders the items, however the background/overlay has an incorrect TPage position, pointing to `(0,0)` instead of `(576,240)`
* [X] Movement tied to FPS
* [X] Rain heightmap indexing produces incorrect results after loading chunks when moving.
* [ ] Cull faces on chunk edges that face outward on the render limit
* [ ] Textures in terrain tpage with any `u` and `v >= 16` rendering multiple interleaved textures from different points in terrain texture page (NOTE: The issue seems to be specific to `POLY_FT4` since `POLY_GT4` doesn't have this behaviour when I changed the primitive type)
* [ ] Mesh vertices z-depth is inconsistent leading to faces drawn in wrong order and thus culling fails
* [ ] Vertices are distorted (in their location) when very close to the camera
* [ ] Move mesh generation to after all loading when updating world to avoid face generation on orthogonal axis to update axis
* [ ] Lighting on dropped items is pure black sometimes despite being in light (possibly bad world position when retrieving light value). The physics object position for items isn't properly aligned to the bounding box since the position (which should be the centre of the AABB) isn't aligned properly and thus when converting to world position and querying the light level, it can query the next block over (in the direction that the item moved when it was dropped) and thus can get a light level of 0 and the item is rendered as black in the world.
* [ ] Weather texture does not scroll correctly, some planes are static and only in some places does it work correctly. Potentially an issue with texture windowing and UV positions.
* [ ] Holding movement keys and opening the inventory doesn't prevent player from continuing to move.

## Implement

* [X] Implement texture loading via LZP compressed archive traversal
* [X] Complete display/render context implementation
* [X] Create standalone cube struct and load + render methods
* [X] Greedy meshing for chunk rendering
* [X] Create loading screen to visualise loading progress. Something simple like a loading bar for now. Make it more fancy at a later point.
* [X] Continuous shifting world chunk array
* [X] Graphing lib to use for resource usage and debugging
* [X] Ray cast from camera normal to block retrieving exact block coordinates in world
* [X] Dynamic chunk loading and unloading with movement
* [X] Player entity with hit box
* [X] Basic entity-mesh interaction physics with gravity
* [X] Crosshair
* [X] Axis marker
* [X] Proper engine structure with logic interface
* [X] Inheritable Block interface with concrete implementations
* [X] 2D array indexed terrain texture specification in face attributes
* [X] Support transparent textures (original and binary greedy meshing)
* [X] Full ASCII, extended characters and symbols set front + printing lib
* [X] Shadowed version of font to use via flag for printing
* [X] Breaking overlays for a block, enabled with a marker variable and a target resolved by ray cast and a texture reference
* [X] Finish player attack input handler
* [X] Add handling for player use handler with logic for using items and applying damage on items like fishing rods
* [X] Hotbar active slot input handlers (left/right)
* [X] Set block orientation relative to face placed against
* [X] Lightmapping with flood fill algorithm using LUTs for block light (adding and removing)
* [X] Add lightmap indexes into mesh generated from BGM
* [X] Consider `opacity_bitmap` for blocks that are true for `blockCanLightNoPropagate(id)`
* [X] Initial sunlight propagation for chunks
* [X] ~~Procedural texturing for each quad in chunk mesh by rendering texture and light maps to off screen TPage and then using that to render to world.~~
* [X] Bake lighting into mesh
* [X] ~~Do a mini version of the binary greedy meshing for lightmapping to generate a cached entry on each mesh primitive taht is just a 2D array of final light levels that can be applied when rendering the overalys as `TILE_16` by looping over the texture coords (x,y) and indexing this cached map~~ (Lighting now baked into mesh, this isn't needed)
* [X] Day/night cycle does not need to have lighting recalculated for each time the lighting changes. Everything can be static, instead the skylight 4 bits of the lightmap entries applied to mesh quads is capped based on the time of day before we determine the light value between the skylight upper 4 bits and block light lower 4 bits. This allows for seemless day/night transitions in lightlevels without needing to recalculate.
* [X] Hold an internal global light level on the world that changes with time to be used in adjusting sky lighting when it changes
* [X] Remove block sunlight updates
* [X] Memory usage stats in debug UI overlay
* [X] Add `PhysicsObject` and `Entity` sub-structues to `Item` conditional on being `in_world`. Refactor the rendering to make the item bob only when on the ground and handle initialisation in `modifyVoxel0` function.
* [X] Loading new chunks should only happen when the player is in the bordering chunks and beyond the middle of the chunk (with respect to the centre of the world chunk grid).
* [X] Items that go from one chunk to another (thrown, dropped, block beneath broken, etc) need to transition in ownership from the current chunk to the new chunk.
* [X] Check block place raycast result intersection with player AABB, preventing placement if coordinates overlap.
* [X] Calculate time-of-day as the tick count up to `20t * 20m * 60s = 24000t` ticks  and update world internal light level at various thresholds
* [X] Limit world internal lighting level based on rain and thunder strength.
* [X] Weather cycle at random intervals with varying strength.
* [X] Add snow/rain rendering in scene as several orthogonal planes that are positioned at player position.
* [X] ~~Check if camera not moving then ray cast once to get block targeted and draw outline around if in reach range~~
* [ ] Camera far plane cutoff with fog relative to chunk render distance
* [ ] Frustum and culling
* [ ] Depth-first search culling through chunks
* [ ] Thread scheduler using hardware timers for IRQ interleaved execution
* [ ] Update queue for operations on the world
* [ ] D-pad or analogue stick controlled cursor in UIs with item movement between slots being held by cursor
* [ ] TooManyItems-like overlay in inventory GUI
* [ ] Retrieve face attributes based on block orientation during meshing
* [ ] Sub-block intersection tests for raycast to handling interacting with blocks like doors and piston heads
* [ ] If we are breaking a block on the boundary of chunks, we should pass the breaking context to both the target and neighbouring chunk in order for the mesh generation to account for the missing faces on the chunk boundary.
* [ ] Add support in chunk provider for providing a list of named stages for chunk loading (e.g. gen terrain, prop lighting, construct mesh), then use these for dynamically loading chunks as well as normal world initialisation.
* [ ] Support more general mesh generation based on block types including multiple normals
* [ ] Block light should have a slight tint towards red for a warmer colour.
* [ ] Support smooth lighting by optionally switching to an alternate `ChunkMesh` render handler that uses `POLY_GT4`. Vertex colours should be calculated by querying the light level for each vertex (with some adjustment for direction since left to right is not the same as right to left when getting lighting values).
* [ ] Sound for rain as inside/outside variant should be based on whether player is below the top block in the heightmap. More specifically within a chunk that is at or below the top and within a some distance from nearest exposed (to top) blocks.
* [ ] Handle non-uniform block models when generating meshes (i.e. stairs), including orientation where only some faces can be merged.
* [ ] Polygon subdivision (polygons of minimum size, i.e. block size) for the mesh of the chunk that the player resides within
* [ ] Recipe compiler to take JSON format and produce compile time struct definition encapsulating recipes

## Refactor

* [X] Support texture windowing for wrap-around in multiples of texture width/height
* [X] Removed aliasing wrappers for primitive/vertex/normal arrays in `ChunkMesh` and refactor to direct type alias of `SMD`
* [X] Camera displayed position to be units of `BLOCK_SIZE`
* [X] Y-axis values are negative upwards should be positive
* [X] Block world coords should start at 0 from bottom of chunk and go up positively.
* [X] Move crosshair render handler to UI directory with dependent structure
* [X] Move axis render handler to UI directory with dependent structure
* [X] Create loading screen structure with update/render methods and move content in world to there
* [X] Ray cast should check distance walked is less than radius instead of out of world check
* [X] Use geometry matrix to centre loading screen text and loading bar instead of manual positioning
* [X] Refactor `World` and `Chunk` to support interface based blocks with dynamic dispatch handlers
* [X] Move to binary greedy meshing with bitwise ops for single-pass mask creation
* [X] Migrate to chunk providers and generators created with world for specific generation
* [X] Move old mesh generation in `Chunk` into standalone implementation in `src/game/world/chunk/meshing` or remove
* [X] Per-face opacity control on each block for minimal meshing with mixed opacity (i.e. like farmland or stairs)
* [X] Breaking overlay should mark target block as being broken, have a global to mark the block being broken and the breaking texture index to use, then trigger a chunk mesh regeneration on starting to break the block and when stopped (not finished breaking). Render the overlay on top of the target texture to a part of the framebuffer with `(16*6)x16` reserved to merge the texture and overlay together. Then ensure the block render tpage is poiting to the reserved area by `face_dir * 16` for that face (in the mesh data).
* [X] Move static values like block/item names and max stack sizes to global constants accessed by ID
* [X] Merge both font textures (with and without shadow) into a single texture to use space more efficiently and avoid the gap between the current texture file layout in VRAM.
* [X] Use custom mesh format for chunk meshes to avoid excessive overhead of unused fields with SMD structured
* [X] Return `NULL` block/chunk when accessing block/chunk outside of loaded range
* [X] Move light/add remove to be standalone methods to enqueue updates then have a separate chunk-internal method to process the lighting updates during a `chunkUpdate` call
* [X] Light updates should be a queue with size limited to `CHUNK_SIZE ^ 3` (cubed) and ordered on recency of push. If a new update is going to overwrite and old one (at the same index) then the `max(old_light_level, new_light_level)` should be used as the value for addition and the `min(...)` for removal. Can probably just add forward/backward pointers to hashmap bucket implementation for this. (Just using a hashmap keyed on pos vector was sufficient)
* [X] Change default return of `15` to world light in out-of-bounds cases for world and chunk light retrieval for the sky subset of bits only.
* [X] Change light levels scalar (calculated) to return values that scale down by `80%` on each level. I.e. level 14 is 80% of level 15, level 13 is 80% of level 14, etc.
* [X] ~~Move rendering handlers in `ChunkMesh` to standalone SMD renderer file~~
* [X] Move remesh trigger handling for lighting and breaking overlay from `chunkRender` into `chunkUpdate` so that changes that don't directly invoke a re-mesh, but stil need one can do it in an update cycle
* [X] Move to PSn00bSDK critical handlers and properly mark initialisation of timers.
* [X] ~~Refactor vector operations to use `_Generic` C11 macro to perform type specific operations between any kind of two vector types or constant.~~ (Refactor was done with vec/const specific macros on naming basis).
* [X] Move field `face_attributes` in `Block` to `BlockAttributes`, moving from a fixed sized array to a pointer, allowing for variable length arrays indexed by `metadata_id * FACE_DIRECTION_COUNT`. This can then be an alias to an array of texture refs that all blocks of the same type can use and simultaneously allow for many variants based on `metadata_id`.
* [X] Move assets to on-disk directories and files instead of packing them into the binary
* [X] Move dynamic assets (like GUIs) into separate on-disk LZP archive to be referenced ad-hoc instead of needing to keep entire assets resource in memory all time time including static assets.
* [ ] Support other resolutions that aren't 320x240
* [ ] Refactor the `chunkRemoveLightType` call into the `chunkSetLightType` when the light value is `0` and update the necessary logic changes to accomodate this (seems to cause infinite lighting update loops if this is done at the moment)
* [ ] Change block equality check to account for both `id` and `metadata_id` fields in all relevant places (i.e. binary greedy mesher)
* [ ] Replace fixed dual `LINE_F2` crosshair with texture rendered from GUI texture, allowing user customisation and saving an OT entry + draw call
