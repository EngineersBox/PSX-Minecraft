#include "menus.h"

#include "menu_id.h"
#include "main.h"
#include "singleplayer.h"

const MenuConstructor menu_constructors[MENU_COUNT] = {
    [MENUID_MAIN] = mainMenuNew,
    [MENUID_SINGLEPLAYER] = singleplayerMenuNew,
};

const MenuDestructor menu_destructors[MENU_COUNT] = {
    [MENUID_MAIN] = mainMenuDestroy,
    [MENUID_SINGLEPLAYER] = singleplayerMenuDestroy,
};
