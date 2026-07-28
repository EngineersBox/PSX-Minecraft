# Frustum Culling

TODO: Write this doc and a blog bost

* Traditionally frustum has two FOV angles over X and Y to fit exactly against a 16:9, 16:10, etc (rectangular) window.
* traditional frustums are made of planes (point and normal vector), where either planes are transformed inversely to camera or the world geometry is transformed before checking where geometry is in the frustum.
* This process is costly and requires at best 1 check and at worst 2 for each plane (depending on whether you consider near-far separately or not). This is excessive for a PS1
* Instead we can use some nice properties of frustums to simplify the logic. Note a frustum can be defined with four infinite planes, each pair with angle between them.
* Each pair of planes (opposing) are orthogonal or the other pair, so we can split the frustum check into two checks for each angle.
* This aovids annoyingly expensive computation or precalculated LUTs that occupy memory.

* Instead of traditional trigonometry, we can use taxicab/Manhattan geometry to avoid trig functions (acos, arctan2, etc), this essentially means the unit circle is now a unit diamond.
* This allows us to work with taxicab trigonometry which are simple operations (i.e. division for projection)
* Angles in this space are measured in t-rads which are the units of arc lengths subtentended by an angle, essentially radians become t-radians.
* Thus, we can convert the camera rotation vector (yaw & pitch) into t-rad angles for X-Z (pitch) and X-Y (yaw) simply by shifting right by 10.
* Then calculate the relative direction vector from  thev player to queried chunk (in units of chunks and not blocks to keep the values small in Q12 fixed point) and compute t-rad angle for Z-Y, Z-X
* Now we can determine if the chunk angles are within FOV / 2 of the camera facing angles for culling queries
* If either of queried chunk angles are outside of this range, then it can be culled.

* This has some problems, such as if both the centre and vertices of a chunk are not visible to the frustum, but a subsetion of it is.
* We need a range query approach that can consider the entire angle span a chunk occupies relative to the player.
* By choosing verticies that are in opposing corners (i.e. (0,0) and (1,1)) based on the quadrant relative to the player position, we can compute a range span with two t-rad angles for each vertex.
* Using these, we compute the range overlap with the frustum t-rad angles to determine visibility.
* Due to the independence of the Z-Y, Z-X planes this ensures two tests can fully determine frustum overlap.
