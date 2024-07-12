#include "block.h"

bool iBlockUseAction(VSelf) ALIAS("IBlock_useAction");
bool IBlock_useAction(VSelf) {
    // By default blocks don't react to being interacted with
    return false;
}
