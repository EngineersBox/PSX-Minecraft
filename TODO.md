# TODO

## Implement

* [x] Implement texture loading via LZP compressed archive traversal
* [x] Complete display/render context implementation
* [x] Create standalone cube struct and load + render methods
* [x] Greedy meshing for chunk rendering
* [ ] Create loading screen to visualise loading progress. Something simple like a loading bar for now. Make it more
  fancy at a later point.
* [ ] Camera far plane cutoff with fog relative to chunk render distance

## Refactor

* [x] Support texture windowing for wrap-around in multiples of texture width/height
* [ ] Move rendering handlers in ChunkMesh to standalone SMD renderer file

## Fix

* [x] Fix chunk mesh traversal to be Y up instead of Z up
* [ ] Fix last vertex in chunk not correct
* [x] Fix last texture window causing font render to be weird
* [ ] Figure out why odd numbered chunks render, no x-valued chunks render and Y alternate chunks don't render
* [ ] Figure out why CHUNK_SIZE that isn't 8 doesn't work (e.g. 16), in that nothing renders (no errors logged)
* [x] Fix mesh rendering of last faces connected to neighbouring chunk, normal seems inverted. 
  This might be the inverse of changes made with windowing and indexing to fix chunk rendering initially.
* [ ] Camera rotation values do not wrap when a complete rotation is achieved