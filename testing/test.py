from enum import Enum
from dataclasses import dataclass
from collections import deque

CHUNK_SIZE = 4
VOXELS = [
    0, 1, 1, 1,
    1, 1, 1, 0,
    1, 1, 1, 0,
    0, 1, 1, 1,

    1, 0, 0, 1,
    0, 0, 1, 1,
    0, 1, 1, 1,
    0, 0, 0, 1,
    
    1, 1, 1, 0,
    1, 1, 0, 1,
    1, 0, 0, 1,
    0, 1, 1, 0,

    1, 1, 1, 1,
    1, 1, 1, 0,
    1, 1, 1, 0,
    0, 1, 1, 0,
]

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

def main() -> None:
    cluster_index = 0
    clusters: list[Cluster] = [Cluster()]
    block_left = CHUNK_SIZE ** 3
    blocks = deque()
    while (block_left > 0):
        cluster = clusters[cluster_index]


if __name__ == "__main__":
    main()
