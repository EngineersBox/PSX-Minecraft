#include "menus.h"

#include "menu_id.h"

#include "main.h"

const MenuConstructor menu_constructors[MENU_COUNT] = {
    [MENUID_MAIN] = mainMenuNew,
};

const MenuDestructor menu_destructors[MENU_COUNT] = {
    [MENUID_MAIN] = mainMenuDestroy,
};
