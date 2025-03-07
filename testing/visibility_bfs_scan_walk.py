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

def voxel(pos: Pos) -> Optional[int]:
    x, y, z = pos
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

    def getBit(self, pos: Pos) -> int:
        x, y, z = pos
        return (self.bitmap[(y * CHUNK_SIZE) + z] >> x) & 0b1

    def setBit(self, pos: Pos) -> None:
        x, y, z = pos
        self.bitmap[(y * CHUNK_SIZE) + z] |= 1 << x
        print(self.bitmap)

    def clearBit(self, pos: Pos) -> None:
        x, y, z = pos
        self.bitmap[(y * CHUNK_SIZE) + z] &= ~(1 << x);

    def find_unset_pos(self) -> Optional[Pos]:
        for y in range(CHUNK_SIZE):
            for z in range(CHUNK_SIZE):
                x_bits = self.bitmap[(y * CHUNK_SIZE) + z]
                # print(f"X bits: {x_bits:04b}")
                if (x_bits == (2 ** CHUNK_SIZE) - 1):
                    continue
                for x in range(CHUNK_SIZE):
                    if (x_bits & (0b1 << x) == 0 and voxel((x, y, z)) == 0):
                        return (x, y, z)
        return None

class FaceDirection(Enum):
    FACE_DIR_DOWN = 0,
    FACE_DIR_UP = 1,
    FACE_DIR_LEFT = 2,
    FACE_DIR_RIGHT = 3,
    FACE_DIR_BACK = 4,
    FACE_DIR_FRONT = 5

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
        print(f"Offset: {offset}")
        self.bitset |= 0b1 << offset
        print(f"[VIS] Bitset: {self.bitset:015b}")

def main() -> None:
    visibility = chunk_visibility_bfs_walk_scan()
    print(f"{visibility.bitset:015b}")
    pass

def find_root(bitmap: ChunkBitmap) -> Optional[Pos]:
    for y in range(CHUNK_SIZE):
        for z in range(CHUNK_SIZE):
            for x in range(CHUNK_SIZE):
                pos = (x, y, z)
                v = voxel(pos)
                if (v == 0):
                    return pos
                bitmap.setBit(pos)
    return None

def fill_bitmap_solid_direction(bitmap: ChunkBitmap, pos: Pos, dir: Pos) -> int:
    processed = 0
    while (True):
        pos = (
            pos[0] + dir[0],
            pos[1] + dir[1],
            pos[2] + dir[2]
        )
        v = voxel(pos)
        if (v != 1 or bitmap.getBit(pos) == 1):
            # Not solid or already visited
            break
        bitmap.setBit(pos)
        processed += 1
    return processed

def visit_node(pos: Pos, dir: Pos, queue: Deque[Pos], bitmap: ChunkBitmap) -> int:
    next_pos = (
        pos[0] + dir[0],
        pos[1] + dir[1],
        pos[2] + dir[2]
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
    root: Optional[Pos] = find_root(bitmap)
    if (root == None):
        return ChunkVisibility()
    print(f"Root: ({root})")
    queue: Deque[Pos] = deque()
    queue.appendleft(root)
    bitmap.setBit(root)
    # We mark every solid block until the first free block
    # in the bitmap, so we start with already having processed
    # those. This accounts for that, minus an extra 1 for the
    # queued position
    total_blocks_processed = (root[1] * (CHUNK_SIZE ** 2)) + (root[2] * CHUNK_SIZE) + root[0]
    visibility = ChunkVisibility()
    while (total_blocks_processed < CHUNK_SIZE ** 3):
        visible_sides = 0b000000
        while (len(queue) > 0):
            x, y, z = pos = queue.pop()
            total_blocks_processed += 1
            # Left
            if (x == 0):
                visible_sides |= 0b100000
            total_blocks_processed += visit_node(pos, (-1, 0, 0), queue, bitmap)
            # Right
            if (x == CHUNK_SIZE - 1):
                visible_sides |= 0b010000
            total_blocks_processed += visit_node(pos, (+1, 0, 0), queue, bitmap)
            # Front
            if (z == 0):
                visible_sides |= 0b001000
            total_blocks_processed += visit_node(pos, (0, 0, -1), queue, bitmap)
            # Back
            if (z == CHUNK_SIZE - 1):
                visible_sides |= 0b000100
            total_blocks_processed += visit_node(pos, (0, 0, +1), queue, bitmap)
            # Down
            if (y == 0):
                visible_sides |= 0b000001
            total_blocks_processed += visit_node(pos, (0, -1, 0), queue, bitmap)
            # Up
            if (y == CHUNK_SIZE - 1):
                visible_sides |= 0b000010
            total_blocks_processed += visit_node(pos, (0, +1, 0), queue, bitmap)
        print(f"Visibile sides: {visible_sides:06b}")
        if (is_power_of_two(visible_sides)):
            pos = bitmap.find_unset_pos()
            if (pos != None):
                print(f"Unset pos: ({pos})")
                queue.appendleft(pos)
                # Only 0 or 1 bit set, no visibility
                continue
            else:
                print("No unset pos")
                break
        for i in range(6):
            if (visible_sides & (0b1 << i) == 0):
                continue
            for j in range(i + 1, 6):
                if (visible_sides & (0b1 << j) == 0):
                    continue
                print(f"Visible {i} & {j}")
                visibility.setBitLiteral(i, j)
        pos = bitmap.find_unset_pos()
        if (pos == None):
            break
        queue.appendleft(pos)
        print(f"Unset pos 2: ({pos})")
    print(f"Total blocks processed: {total_blocks_processed}/{CHUNK_SIZE ** 3}")
    return visibility


if __name__ == "__main__":
    main()
