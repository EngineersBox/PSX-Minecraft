# Frustum Culling

TODO: Write this doc and a blog bost

* Traditionally frustum has two FOV angles over X and Y to fit exactly against a 16:9, 16:10, etc (rectangular) window.
* traditional frustums are made of planes (point and normal vector), where either planes are transformed inversely to camera or the world geometry is transformed before checking where geometry is in the frustum.
* This process is costly and requires at best 1 check and at worst 2 for each plane (depending on whether you consider near-far separately or not). This is excessive for a PS1
* PS1 uses 4:3 ratios which are essentially square, so we can do away with trapezoidal frustum shapes and instead approximate with a cone, which has only one parameter, an angle theta from the central axis of the cone.
* The cone point/peak is the camera location here.
* This allows us to simplify the visiblity check to a relative angle check to the camera. It covers all angles simultaneously.
* To find a relative angle of a point in 3D space requires some annoyingly expensive computation or precalculated LUTs that occupy memory. Both of these are poor tradeoffs for a PS1 game.

* Instead we can use taxicab/Manhattan geometry to avoid trig functions (acos, arctan2, etc), this essentially means the unit circle is now a unit diamond.
* This allows us to work with taxicab trigonometry which are simple operations (i.e. division for projection)
* Angles in this space are measured in t-rads which are the units of arc lengths subtentended by an angle, essentially radians become t-radians.
* Thus, we can convert the camera facing drection vector into two t-rad angles for X-Y (pitch) and X-Z (yaw)
* Then calculate the relative direction vector from  thev player to queried chunk (in units of chunks and not blocks to keep the values small in Q12 fixed point) and compute t-rad angle for X-Y, X-Z
* Now we can determine if the chunk angles are within FOV / 2 of the camera facing angles for culling queries
* If either of queried chunk angles are outside of this range, then it can be culled.
