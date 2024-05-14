#include "block.h"

bool iBlockIsOpaque(VSelf, FaceDirection face_dir) ALIAS("IBlock_isOpaque");
bool IBlock_isOpaque(VSelf, UNUSED FaceDirection face_dir) {
    return true;
}

u8 iBlockOpaqueBitset(VSelf) ALIAS("IBlock_opaqueBitset");
u8 IBlock_opaqueBitset(VSelf) {
    return 63; // 0b00111111
}