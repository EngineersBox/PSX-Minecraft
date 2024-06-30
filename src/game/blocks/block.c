#include "block.h"

bool iBlockUseAction(VSelf) ALIAS("IBlock_useAction");
bool IBlock_useAction(VSelf) {
    // By default blocks don't react to being interacted with
    return false;
}

bool iBlockIsOpaque(VSelf, FaceDirection face_dir) ALIAS("IBlock_isOpaque");
bool IBlock_isOpaque(VSelf, UNUSED FaceDirection face_dir) {
    return true;
}

u8 iBlockOpaqueBitset(VSelf) ALIAS("IBlock_opaqueBitset");
u8 IBlock_opaqueBitset(VSelf) {
    return 63; // 0b00111111
}