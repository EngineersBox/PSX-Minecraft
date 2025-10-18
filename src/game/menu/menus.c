#include "menus.h"

#include "menu_id.h"

#include "main.h"

MenuConstructor menu_constructors[MENU_COUNT] = {
    [MENUID_MAIN] = mainMenuNew,
};

MenuDestructor menu_destructors[MENU_COUNT] = {
    [MENUID_MAIN] = mainMenuDestroy,
};
