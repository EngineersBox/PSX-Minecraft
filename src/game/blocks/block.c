#include "block.h"

void iblockUpdate(VSelf) ALIAS("IBlock_update");
void IBlock_update(VSelf) {
    // Do nothing
}

bool iBlockUseAction(VSelf) ALIAS("IBlock_useAction");
bool IBlock_useAction(VSelf) {
    // By default blocks don't react to being interacted with
    return false;
}
