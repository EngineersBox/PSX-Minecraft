## Motivation

I am currently utilising the implementation of the Binary Greedy Mesher from this repo for a re-write of Minecraft for the Playstation 1 (yep that one). Mine is translated into C, but is essentially a copy-and-paste of this repo's `greedy_mesher_optimized.rs`.

At some later stage I will add blocks that will support partial transparency in the full texture (such as glass) and blocks with opaque textures with gaps (such as the upper half of doors or pistons). This necessitates transparency support, which the old mesher I have written, has. But using your new fancy, shiny and *blazingly fast* mesher, I want to support transparency there. So, here... we... go.

## Context

Creating a mask of the voxels in the scene is done on an absolute basis, i.e. whether there is or isn't a block, marked by a bit on the relevant query axis in the mask array `col_face_masks`. This is generated using the neat shifting for left/right faces in the cull loop with:

```rust
// set if current is solid and next is air.
let col = axis_cols[axis][z][x];
// sample descending axis, and set true when air meets solid
col_face_masks[2 * axis + 0][z][x] = col & !(col << 1);
// sample ascending axis, and set true when air meets solid
col_face_masks[2 * axis + 1][z][x] = col & !(col >> 1);
```

The last ops ensure that only the bits on the outer faces of a sequence of 1's is set, specifically based on the shift direction. So for instance for a slice (where `0` = air and `1` = block):

<table>
	<tr>
		<td>Slice</td>
		<td>Left Faces</td>
		<td>Right Faces</td>
	</tr>
	<tr>
		<td>
<pre>
00000
00110
00110
01110
01111
</pre>
		</td>
		<td>
<pre>
00000
00100
00100
01000
01000
</pre>
		</td>
		<td>
<pre>
00000
00010
00010
00010
00001
</pre>
		</td>
	</tr>
</table>

Great, this then gets used to build the plane data by creating hashmap entries for the voxel data for each set bit by retrieving the y (axial-specific) value by invoking u64::trailing_zeros() and combining with the x,z iterator values we are traversing through.

## The Problem

We need some way of determining that for any given set bit, there is a following bit (relative to the direction that the face was generated for, by the shift direction) that is still visible through some form of transparency.

More precisely, we want to be able to detect sequential bits that are pairings of solid and transparent voxels and include them both. Let's take an example where `0` = air, `1` = transparent and `2` = solid.

Let's suppose we have the following slice of a chunk, which covers all the cases we need to handle:

```rust
01120
01010
02120
01210
01222
```

Given that we have transparent voxels, we need a way to generate the following masks for the faces in each of the rows (called `col` in code but easier to understand as rows since it's a direct map to binary visually):

<table>
	<tr>
		<td>Left Faces</td>
		<td>Right Faces</td>
	</tr>
	<tr>
		<td>
<pre>
01010
01010
01000
01100
01100
</pre>
		</td>
		<td>
<pre>
00010
01010
00010
00110
00001
</pre>
		</td>
	</tr>
</table>

Take a minute to see why the bits that are set as they are, we need anything solid followed by a transparent voxel in the direction of the face to be included. This begs the question... why do we want this? It is because the meshing plane construction for each face considers each bit of the column independently (assuming only one bit surrounded by zeros) by invoking `u64::trailing_zeros`. However the nature of the implementation means that if there are two *successive* 1 bits then it will consider each distinctly in mesh creation, which allows us to do this transparent-solid handling in any situation.

## Solution

Taking a step back for a second, we can see that the essence of what we are trying to do here is actually detect any voxel that isn't empty as one mask (NE) and then detect any voxel that ins't air and isn't transparent as another mask (NET), then combine them in some manner.

...What?

Let's take our previous example where `0` = air, `1` = transparent and `2` = solid.

```rust
01120
01010
02120
01210
01222
```

Suppose we construct two initial mappings of this slice for each the types mentioned before (NE and NET). Let's do this based on the conditionals mentioned:

<table>
	<tr>
		<td>Non-Empty (NE)</td>
		<td>Non-Empty & Non-Transparent (NET)</td>
	</tr>
	<tr>
		<td>
<pre>
01110
01010
01110
01110
01111
</pre>
		</td>
		<td>
<pre>
00010
00000
01010
00100
00111
</pre>
		</td>
	</tr>
</table>

In terms of implementation it looks like this:

```rust
// solid binary for each x,y,z axis
let mut axis_cols = [[[0u64; CHUNK_SIZE_P]; CHUNK_SIZE_P]; 3];
// solid and non-transparent binary for each x,y,z axis
let mut axis_cols_transparent = [[[0u64; CHUNK_SIZE_P]; CHUNK_SIZE_P]; 3];
// the cull mask to perform greedy slicing, based on solids on previous axis_cols
let mut col_face_masks = [[[0u64; CHUNK_SIZE_P]; CHUNK_SIZE_P]; 6];
#[inline]
fn add_voxel_to_axis_cols(
    b: &crate::voxel::BlockData,
    x: usize,
    y: usize,
    z: usize,
    axis_cols: &mut [[[u64; 34]; 34]; 3],
    axis_cols_transparent: &mut [[[u64; 34]; 34]; 3],
) {
    if !b.block_type.is_solid() {
        return;
    }
    // x,z - y axis
    axis_cols[0][z][x] |= 1u64 << y as u64;
    // z,y - x axis
    axis_cols[1][y][z] |= 1u64 << x as u64;
    // x,y - z axis
    axis_cols[2][y][x] |= 1u64 << z as u64;
    if !b.block_type.is_transparent() {
        // x,z - y axis
        axis_cols_transparent[0][z][x] |= 1u64 << y as u64;
        // z,y - x axis
        axis_cols_transparent[1][y][z] |= 1u64 << x as u64;
        // x,y - z axis
        axis_cols_transparent[2][y][x] |= 1u64 << z as u64;
    }
}
```

We can combine these two *views* in order to get a complete understanding of the overlapping sections, which will indicate where we need to include transparent faces. This is simply a logical AND operation between the two views on a per column basis! (Call this result NETC - NET Combined)

<table>
	<tr>
		<td>Non-Empty (NE)</td>
		<td>Non-Empty & Non-Transparent (NET)</td>
		<td>Non-Empty & Non-Transparent (NETC)</td>
	</tr>
	<tr>
		<td>
<pre>
01110
01010
01110
01110
01111
</pre>
		</td>
		<td>
<pre>
00010
00000
01010
00100
00111
</pre>
		</td>
		<td>
<pre>
00010
00000
01010
00100
00111
</pre>
		</td>
	</tr>
</table>

Using these two tables we can simply repeat the same shifting operations for left and right face detection (left: `col & !(col >> 1)`, right: `col & !(col << 1)`) for both NE and NETC (we don't care about NET since it was used to construct NETC). This provides us a with a visualisation of visible faces for both solid and transparent voxels simultaneously. Using our example, we can see that the following face mappings are generated:

<table>
	<tr>
		<td>NE</td>
		<td>NETC</td>
	</tr>
	<tr>
		<td>
			<table>
				<tr>
					<td>Left Face</td>
					<td>Right Face</td>
				</tr>
				<tr>
					<td>
<pre>
01000
01000
01000
01000
01000
</pre>
				</td>
					<td>
<pre>
00010
00010
00010
00010
00001
</pre>
					</td>
				</tr>
			</table>
		</td>
		<td>
			<table>
				<tr>
					<td>Left Face</td>
					<td>Right Face</td>
				</tr>
				<tr>
					<td>
<pre>
00010
00000
01010
00100
00100
</pre>
					</td>
					<td>
<pre>
00010
00000
01010
00100
00001
</pre>
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>

We are so very close to our final map of all faces with proper transparency handling. Thankfully, the last step is just as simple as the construction of NETC. We just need to apply logical OR between the left and right maps of NE and NETC (i.e. `NE_L | NETC_L` and `NE_R | NETC_R`).

<table>
	<tr>
		<td>Left Face</td>
		<td>Right Face</td>
	</tr>
	<tr>
		<td>
<pre>
01010
01000
01010
01100
01100
</pre>
		</td>
		<td>
<pre>
00010
00010
01010
00110
00001
</pre>
		</td>
	</tr>
</table>

This finalised result can be added as the value for the current column face mask, corresponding to the following implementation:

```rust
// face culling
for axis in 0..3 {
    for z in 0..CHUNK_SIZE_P {
        for x in 0..CHUNK_SIZE_P {
            // set if current is solid, and next is air
            let col = axis_cols[axis][z][x];
            // set if current is solid and not transparent and next is air
            let col_transparent = axis_cols_transparent[axis][z][x];
            // solid
            let solid_descending = col & !(col << 1);
            let solid_ascending = col & !(col >> 1);
            // Transparent
            let transparent_descending = col_transparent & !(col_transparent << 1);
            let transparent_ascending = col_transparent & !(col_transparent >> 1);
            // Combined solid + transparent faces
            col_face_masks[2 * axis + 0][z][x] = solid_descending | transparent_descending;
            col_face_masks[2 * axis + 1][z][x] = solid_ascending | transparent_ascending;
        }
    }
}
```

**B E H O L D**. We have achieved greatness. Now, subsequent (unchanged) plane mesh generation loops will do all the work for us:

```rust
// skip padded by adding 1(for x padding) and (z+1) for (z padding)
let mut col = col_face_masks[axis][z + 1][x + 1];
// removes the right most padding value, because it's invalid
col >>= 1;
// removes the left most padding value, because it's invalid
col &= !(1 << CHUNK_SIZE as u64);
while col != 0 {
    let y = col.trailing_zeros();
    // clear least significant set bit
    col &= col - 1;
    // get the voxel position based on axis
    let voxel_pos = match axis {
        0 | 1 => ivec3(x as i32, y as i32, z as i32), // down,up
        2 | 3 => ivec3(y as i32, z as i32, x as i32), // left, right
        _ => ivec3(x as i32, z as i32, y as i32),     // forward, back
    };
	// ... snip ao ...
    let current_voxel = chunks_refs.get_block_no_neighbour(voxel_pos);
    // let current_voxel = chunks_refs.get_block(voxel_pos);
    // we can only greedy mesh same block types + same ambient occlusion
    let block_hash = ao_index | ((current_voxel.block_type as u32) << 9);
    let data = data[axis]
        .entry(block_hash)
        .or_default()
        .entry(y)
        .or_default();
    data[x as usize] |= 1u32 << z as u32;
}
```

Note **specifically**, that we get the trailing zeros as the y offset for this voxel and then clear **ONLY** this current voxel while creating the entry. The voxel position is queried in the world and we subsequently create a hashmap entry making this possible. Simple.

## Conclusion

Now, theres an obvious caveat to this... you need to implement your mesh generation and subsequently the rendering pipeline such that transparency ordering is respected from the Z-axis (presumably through depth testing) here in order make use of this. This will however, guarantee that the absolute minimum amount of transparent faces are constructed in the mesh.
