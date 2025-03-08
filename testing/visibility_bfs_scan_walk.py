import math
from collections import deque
from typing import Deque, Optional, NamedTuple

CHUNK_SIZE = 4

class Vector(NamedTuple):
    x: int
    y: int
    z: int

class Cluster:
    blocks: list[Vector] = []
    visible_sides: int = 0b000000

VOXELS = [
    1, 0, 0, 0,
    0, 0, 0, 1,
    0, 0, 0, 1,
    1, 0, 0, 0,

    0, 1, 1, 0,
    1, 1, 0, 0,
    1, 0, 0, 0,
    1, 1, 1, 0,
    
    0, 0, 0, 1,
    0, 0, 1, 0,
    0, 1, 1, 0,
    1, 0, 0, 1,

    0, 0, 0, 0,
    0, 0, 0, 1,
    0, 0, 0, 1,
    1, 0, 0, 1,
]

def voxel(pos: Vector) -> Optional[int]:
    if (pos.x < 0 or pos.x >= CHUNK_SIZE):
        return None
    elif (pos.y < 0 or pos.y >= CHUNK_SIZE):
        return None
    elif (pos.z < 0 or pos.z >= CHUNK_SIZE):
        return None
    return VOXELS[(pos.y * (CHUNK_SIZE ** 2)) + (pos.z * CHUNK_SIZE) + pos.x]

class ChunkBitmap:
    bitmap: list[int]

    def __init__(self):
        self.bitmap = [0] * (CHUNK_SIZE ** 2)

    def getBit(self, pos: Vector) -> int:
        x, y, z = pos
        return (self.bitmap[(y * CHUNK_SIZE) + z] >> x) & 0b1

    def setBit(self, pos: Vector) -> None:
        self.bitmap[(pos.y * CHUNK_SIZE) + pos.z] |= 1 << pos.x

    def clearBit(self, pos: Vector) -> None:
        self.bitmap[(pos.y * CHUNK_SIZE) + pos.z] &= ~(1 << pos.x);

    def find_unset_pos(self) -> Optional[Vector]:
        for y in range(CHUNK_SIZE):
            for z in range(CHUNK_SIZE):
                x_fwd_bits = self.bitmap[(y * CHUNK_SIZE) + z]
                fwd_some_set = x_fwd_bits != (2 ** CHUNK_SIZE) - 1
                x_bwd_bits = self.bitmap[((CHUNK_SIZE - 1 - y) * CHUNK_SIZE) + (CHUNK_SIZE - 1 - z)]
                bwd_some_set = x_bwd_bits != (2 ** CHUNK_SIZE) - 1
                if (not fwd_some_set and not bwd_some_set):
                    continue
                i = y + z
                if (fwd_some_set and bwd_some_set):
                    for x in range(CHUNK_SIZE):
                        if (i + x % 2 != 0):
                            pos = Vector(
                                CHUNK_SIZE - 1 - x,
                                CHUNK_SIZE - 1 - y,
                                CHUNK_SIZE - 1 - z
                            )
                            if (x_bwd_bits & (0b1 << x) == 0 and voxel(pos) == 0):
                                return pos
                        else:
                            pos = Vector(x, y, z)
                            if (x_fwd_bits & (0b1 << x) == 0 and voxel(pos) == 0):
                                return pos
                if (fwd_some_set):
                    x_bits = x_fwd_bits
                    start = 0
                    end = CHUNK_SIZE
                else:
                    x_bits = x_bwd_bits
                    start = CHUNK_SIZE - 1
                    end = -1
                for x in range(start, end):
                    pos = Vector(x, y, z)
                    if (x_bits & (0b1 << x) == 0 and voxel(pos) == 0):
                        return pos
        return None

class ChunkVisibility:
    # Direction bitmask
    # LRFBUD   Val   Idx <- AKA log2(Val)
    # 000001 ->  1 -> 0 = D
    # 000010 ->  2 -> 1 = U
    # 000100 ->  4 -> 2 = B
    # 001000 ->  8 -> 3 = F
    # 010000 -> 16 -> 4 = R
    # 100000 -> 32 -> 5 = L
    #
    #    Bitmask   Val   Idx   Num
    # LR 110000 -> 48 -> 14  -+
    # LF 101000 -> 40 -> 13   |
    # LB 100100 -> 36 -> 12   | 5
    # LU 100010 -> 34 -> 11   |
    # LD 100001 -> 33 -> 10  -+
    # RF 011000 -> 24 ->  9  -+
    # RB 010100 -> 20 ->  8   | 4
    # RU 010010 -> 18 ->  7   |
    # RD 010001 -> 17 ->  6  -+
    # FB 001100 -> 12 ->  5  -+
    # FU 001010 -> 10 ->  4   | 3
    # FD 001001 ->  9 ->  3  -+
    # BU 000110 ->  6 ->  2  -+ 2
    # BD 000101 ->  5 ->  1  -+
    # UD 000011 ->  3 ->  0   ] 1
    # 
    # The combined bitmask is just a monotonic
    # series of sequential integers. The offset
    # to any of the sections of the bitmask sections
    # is ((n - 1) * n) >> 1, where n is the max(a,b)
    # of the two directions in question. We then
    # just add the min(a,b) of of these directions
    # onto that value to get the bit index into
    # the bitset.
    # 
    # I.e. for directions L and B with directional
    # bitmasks of 100000 and 000100 repectively, this
    # is:
    # max(5, 2) => 5
    # min(5, 2) => 2
    # ((5 - 1) * 5) >> 2 => 10
    # therefore bit index = 10 + 2 => 12
    bitset = 0b0000_0000_0000_000

    def getBit(self, a: int, b: int) -> int:
        n = int(math.log2(max(a, b)))
        m = int(math.log2(min(a, b)))
        return self.getBitLiteral(n, m)

    def getBitLiteral(self, a: int, b: int) -> int:
        if (a == b):
            return 0
        n = max(a, b)
        m = min(a, b)
        offset = (((n - 1) * n) >> 1) + m
        return (self.bitset >> offset) & 0b1
                
    def setBit(self, a: int, b: int) -> None:
        n = int(math.log2(max(a, b)))
        m = int(math.log2(min(a, b)))
        self.setBitLiteral(n, m)

    def setBitLiteral(self, a: int, b: int) -> None:
        if (a == b):
            return
        n = max(a, b)
        m = min(a, b)
        offset = (((n - 1) * n) >> 1) + m
        self.bitset |= 0b1 << offset

def find_root(bitmap: ChunkBitmap) -> Optional[Vector]:
    for y in range(CHUNK_SIZE):
        for z in range(CHUNK_SIZE):
            for x in range(CHUNK_SIZE):
                pos = Vector(x, y, z)
                v = voxel(pos)
                if (v == 0):
                    return pos
                bitmap.setBit(pos)
    return None

def fill_bitmap_solid_direction(bitmap: ChunkBitmap, pos: Vector, dir: Vector) -> int:
    processed = 0
    while (True):
        pos = Vector(
            pos.x + dir.x,
            pos.y + dir.y,
            pos.z + dir.z
        )
        v = voxel(pos)
        if (v != 1 or bitmap.getBit(pos) == 1):
            # Not solid or already visited
            break
        bitmap.setBit(pos)
        processed += 1
    return processed

def visit_node(pos: Vector, dir: Vector, queue: Deque[Vector], bitmap: ChunkBitmap) -> int:
    next_pos = Vector(
        pos.x + dir.x,
        pos.y + dir.y,
        pos.z + dir.z
    )
    v = voxel(next_pos)
    if (v == None):
        # Outside the chunk
        return 0
    if (bitmap.getBit(next_pos) == 1):
        # Already visited
        return 0
    if (v == 0):
        # Transparent and unvisited
        queue.appendleft(next_pos)
        bitmap.setBit(next_pos)
        return 0
    # Solid and unvisited
    # If we hit a solid block, keep filling the bitmap in that direction
    # which allows us to overlap this operation with the initial bitmap
    # scan that binary greedy meshing needs in PSXMC
    return fill_bitmap_solid_direction(bitmap, pos, dir)

def is_power_of_two(n: int) -> bool:
    return (n != 0) and (n & (n-1) == 0)

def chunk_visibility_bfs_walk_scan() -> ChunkVisibility:
    bitmap = ChunkBitmap()
    root: Optional[Vector] = find_root(bitmap)
    if (root == None):
        return ChunkVisibility()
    print(f"Root: ({root})")
    queue: Deque[Vector] = deque()
    queue.appendleft(root)
    bitmap.setBit(root)
    # We mark every solid block until the first free block
    # in the bitmap, so we start with already having processed
    # those. This accounts for that, minus an extra 1 for the
    # queued position
    total_blocks_processed = (root.y * (CHUNK_SIZE ** 2)) + (root.z * CHUNK_SIZE) + root.x
    visibility = ChunkVisibility()
    clusters: list[Cluster] = []
    while (total_blocks_processed < CHUNK_SIZE ** 3):
        # TODO: Can we XOR the visibility bitset from each cluster,
        #       and only mark side cominations not visible if their
        #       bit pairs are both 1's in the XOR result?
        cluster = Cluster()
        clusters.append(cluster)
        while (len(queue) > 0):
            pos = queue.pop()
            total_blocks_processed += 1
            cluster.blocks.append(pos)
            # Left
            if (pos.x == 0):
                cluster.visible_sides |= 0b100000
            total_blocks_processed += visit_node(pos, Vector(-1, 0, 0), queue, bitmap)
            # Right
            if (pos.x == CHUNK_SIZE - 1):
                cluster.visible_sides |= 0b010000
            total_blocks_processed += visit_node(pos, Vector(+1, 0, 0), queue, bitmap)
            # Front
            if (pos.z == 0):
                cluster.visible_sides |= 0b001000
            total_blocks_processed += visit_node(pos, Vector(0, 0, -1), queue, bitmap)
            # Back
            if (pos.z == CHUNK_SIZE - 1):
                cluster.visible_sides |= 0b000100
            total_blocks_processed += visit_node(pos, Vector(0, 0, +1), queue, bitmap)
            # Down
            if (pos.y == 0):
                cluster.visible_sides |= 0b000001
            total_blocks_processed += visit_node(pos, Vector(0, -1, 0), queue, bitmap)
            # Up
            if (pos.y == CHUNK_SIZE - 1):
                cluster.visible_sides |= 0b000010
            total_blocks_processed += visit_node(pos, Vector(0, +1, 0), queue, bitmap)
        print(f"Visibile sides: {cluster.visible_sides:06b}")
        if (is_power_of_two(cluster.visible_sides)):
            pos = bitmap.find_unset_pos()
            if (pos != None):
                print(f"Found unset pos: ({pos})")
                queue.appendleft(pos)
                # Only 0 or 1 bit set, no visibility
                continue
            else:
                break
        for i in range(6):
            if (cluster.visible_sides & (0b1 << i) == 0):
                continue
            for j in range(i + 1, 6):
                if (cluster.visible_sides & (0b1 << j) == 0):
                    continue
                visibility.setBitLiteral(i, j)
        pos = bitmap.find_unset_pos()
        if (pos == None):
            break
        queue.appendleft(pos)
        print(f"Found unset pos 2: ({pos})")
    print(f"Total blocks processed: {total_blocks_processed}/{CHUNK_SIZE ** 3}")
    print(f"Cluster count: {len(clusters)}")
    return visibility

def main() -> None:
    visibility = chunk_visibility_bfs_walk_scan()
    print(f"{visibility.bitset:015b}")
    pass

if __name__ == "__main__":
    main()
