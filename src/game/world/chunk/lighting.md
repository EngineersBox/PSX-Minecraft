# Lighting

TODO: Explain how lighting works

Explain trade-off between choices:

1. Bake into mesh, need to remesh on sunlight changes at start/end of day and block placement but does not add any rendering overhead
2. Procedural texturing with lighting overlay, very heavy OT and PB usage, heavily limiting rendering, no need to remesh on every sunlight update or block placement

References:
* Block light: <https://www.reddit.com/r/gamedev/comments/2iru8i/fast_flood_fill_lighting_in_a_blocky_voxel_game/>
* Sky light: <https://www.reddit.com/r/gamedev/comments/2k7gxt/fast_flood_fill_lighting_in_a_blocky_voxel_game/>
