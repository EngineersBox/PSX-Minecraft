#include "block.h"

bool iBlockIsOpaque(VSelf, FaceDirection face_dir) __attribute__((alias("IBlock_isOpaque")));
bool IBlock_isOpaque(VSelf, UNUSED FaceDirection face_dir) {
    return true;
}

u8 iBlockOpaqueBitset(VSelf) __attribute__((alias("IBlock_opaqueBitset")));
u8 IBlock_opaqueBitset(VSelf) {
    return 63; // 0b00111111
}