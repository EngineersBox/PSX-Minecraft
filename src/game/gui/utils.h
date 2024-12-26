#pragma once

#ifndef _PSXMC__GAME_GUI__UTILS_H_
#define _PSXMC__GAME_GUI__UTILS_H_

#include "slot.h"

void cursorSplitOrStoreOne(Slot* slot,
                           SlotItemGetter getter,
                           SlotItemSetter setter);

#endif // _PSXMC__GAME_GUI__UTILS_H_
