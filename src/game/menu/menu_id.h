#pragma once

#ifndef _GAME_MENU__MENU_ID_H_
#define _GAME_MENU__MENU_ID_H_

#include "../../util/preprocessor.h"

#define MENU_LIST(f) \
    f(ENUM_ENTRY_ORD(MENUID_MAIN, 0)) \
    f(ENUM_ENTRY_ORD(MENUID_SINGLEPLAYER, 1)) \
    f(ENUM_ENTRY_ORD(MENUID_OPTIONS, 2))

#define MENU_COUNT (MENU_LIST(enumCount) 0)

typedef enum EMenuID {
    MENU_LIST(enumConstruct)
} EMenuID;

#endif // _GAME_MENU__MENU_ID_H_
