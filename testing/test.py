from enum import Enum
import math
from dataclasses import dataclass
from collections import deque
from typing import Deque, Optional, Tuple

CHUNK_SIZE = 4

Pos = Tuple[int, int, int]

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

def voxel(x: int, y: int, z: int) -> Optional[int]:
    if (x < 0 or x >= CHUNK_SIZE):
        return None
    elif (y < 0 or y >= CHUNK_SIZE):
        return None
    elif (z < 0 or z >= CHUNK_SIZE):
        return None
    return VOXELS[(y * (CHUNK_SIZE ** 2)) + (z * CHUNK_SIZE) + x]

class ChunkBitmap:
    bitmap: list[int]

    def __init__(self):
        self.bitmap = [0] * (CHUNK_SIZE ** 2)

    def at(self, x: int, y: int, z: int) -> int:
        return (self.bitmap[(y * CHUNK_SIZE) + z] >> x) & 0b1

    def setBit(self, x: int, y: int, z: int, value: int) -> None:
        if (value == 1):
            self.bitmap[(y * CHUNK_SIZE) + z] |= value << x
        else:
            self.bitmap[(y * CHUNK_SIZE) + z] &= ~(1 << x);

    def find_unset_pos(self) -> Optional[Pos]:
        for y in range(CHUNK_SIZE):
            for z in range(CHUNK_SIZE):
                x_bits = self.bitmap[(y * CHUNK_SIZE) + z]
                if (x_bits == (2 ** 4) - 1):
                    continue
                for x in range(CHUNK_SIZE):
                    if (x_bits & (0b1 << x) > 0 and voxel(x, y, z) == 0):
                        return (x, y, z)
        return None

class FaceDirection(Enum):
    FACE_DIR_DOWN = 0,
    FACE_DIR_UP = 1,
    FACE_DIR_LEFT = 2,
    FACE_DIR_RIGHT = 3,
    FACE_DIR_BACK = 4,
    FACE_DIR_FRONT = 5

@dataclass()
class Cluster:
    blocks = [],
    visible_sides = 0b000000

class ChunkVisibility:
    bitset = 0b0000_0000_0000_000

    def getBit(self, a: int, b: int) -> int:
        n = int(math.log2(max(a, b)))
        m = int(math.log2(min(a, b)))
        return self.getBitLiteral(n, m)

    def getBitLiteral(self, a: int, b: int) -> int:
        n = max(a, b)
        m = min(a, b)
        offset = ((n * (n + 1)) >> 1) + m
        return (self.bitset >> offset) & 0b1
                
    def setBit(self, a: int, b: int) -> None:
        n = int(math.log2(max(a, b)))
        m = int(math.log2(min(a, b)))
        self.setBitLiteral(n, m)

    def setBitLiteral(self, a: int, b: int) -> None:
        n = max(a, b)
        m = min(a, b)
        offset = ((n * (n + 1)) >> 1) + m
        self.bitset &= 0b1 << offset

def main() -> None:
    visibility = chunk_visibility_bfs_walk_scan()
    print(f"{visibility.bitset:015b}")
    pass

def find_root() -> Optional[Pos]:
    for y in range(CHUNK_SIZE):
        for z in range(CHUNK_SIZE):
            for x in range(CHUNK_SIZE):
                v = voxel(x, y, z)
                if (v == 1):
                    return (x, y, z)
    return None

def fill_bitmap_solid_direction(bitmap: ChunkBitmap, pos: Pos, dir: Pos) -> int:
    processed = 0
    while (True):
        pos = (pos[0] + dir[0], pos[1] + dir[1], pos[2] + dir[2])
        v = voxel(*pos)
        if (v != 1):
            break 
        bitmap.setBit(*pos, 1)
        processed += 1
    return processed

def walk_node(pos: Pos, dir: Pos, queue: Deque[Pos], bitmap: ChunkBitmap) -> int:
    v = voxel(*pos)
    if (v == None):
        return 0
    if (bitmap.at(
        pos[0] + dir[0],
        pos[1] + dir[1],
        pos[2] + dir[2]
    ) == 1):
        return 0
    if (v == 1):
        # If we hit a solid block, keep filling the bitmap in that direction
        # which allows us to overlap this operation with the initial bitmap
        # scan that binary greedy meshing needs in PSXMC
        return fill_bitmap_solid_direction(bitmap, pos, dir)
    queue.appendleft(pos)
    return 0

def is_power_of_two(n):
    return (n != 0) and (n & (n-1) == 0)

def chunk_visibility_bfs_walk_scan() -> ChunkVisibility:
    root: Optional[Pos] = find_root()
    if (root == None):
        return ChunkVisibility()
    bitmap = ChunkBitmap()
    queue: Deque[Pos] = deque()
    queue.appendleft(root)
    total_blocks_processed = 0
    visibility = ChunkVisibility()
    while (total_blocks_processed < CHUNK_SIZE ** 3):
        cluster = Cluster()
        while (len(queue) > 0):
            x, y, z = queue.pop()
            bitmap.setBit(x, y, z, 1)
            total_blocks_processed += 1
            # Left
            if (x == 0):
                cluster.visible_sides |= 0b000001
            walk_node((x, y, z), (x - 1, y, z), queue, bitmap)
            # Right
            if (x == CHUNK_SIZE - 1):
                cluster.visible_sides |= 0b000010
            walk_node((x, y, z), (x + 1, y, z), queue, bitmap)
            # Back
            if (z == 0):
                cluster.visible_sides |= 0b000100
            walk_node((x, y, z), (x, y, z - 1), queue, bitmap)
            # Front
            if (z == CHUNK_SIZE - 1):
                cluster.visible_sides |= 0b001000
            walk_node((x, y, z), (x, y, z + 1), queue, bitmap)
            # Down
            if (y == 0):
                cluster.visible_sides |= 0b010000
            walk_node((x, y, z), (x, y - 1, z), queue, bitmap)
            # Up
            if (y == CHUNK_SIZE - 1):
                cluster.visible_sides |= 0b100000
            walk_node((x, y, z), (x, y - 1, z), queue, bitmap)
        if (not is_power_of_two(cluster.visible_sides)):
            # Only 0 or 1 bit set, no visibility
            continue
        for i in range(6):
            if (cluster.visible_sides & (0b1 << i) == 0):
                continue
            for j in range(i, 6):
                if (cluster.visible_sides & (0b1 << j) == 0):
                    continue
                visibility.setBitLiteral(i, j)
    return visibility


if __name__ == "__main__":
    main()
