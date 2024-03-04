#include "block.h"

bool iBlockIsOpaque(VSelf) __attribute__((alias("IBlock_isOpaque")));
bool IBlock_isOpaque(VSelf) {
    return true;
}
