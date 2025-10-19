#include "menu.h"

#include "menus.h"
#include "../../core/input/input.h"
#include "../../logging/logging.h"

IUI* current_menu = NULL;

void menuOpen(EMenuID menu_id) {
    const MenuConstructor ctor = menu_constructors[menu_id];
    IUI* next_menu = ctor();
    assert(next_menu != NULL);
    menuSetCurrent(next_menu);
}

void menuSetCurrent(IUI *menu) {
    if (menuIsOpen()) {
        // Menus act as a single element stack, so
        // the last input handler will alawys be
        // registered to the currently opened menu.
        // This cirumstance occurs between menu
        // transitions
        DEBUG_LOG("[MENU] Closing current menu\n");
        VCALL(*current_menu, close);
        DEBUG_LOG("[MENU] Removing last handler\n");
        inputRemoveLastHandler(&input);
    }
    DEBUG_LOG("[MENU] Setting current menu to new menu\n");
    current_menu = menu;
    if (menuIsOpen()) {
        DEBUG_LOG("[MENU] Registering new menu input handler\n");
        VCALL_SUPER(*menu, IInputHandler, registerInputHandler, &input, NULL);
        DEBUG_LOG("[MENU] Opening new menu\n");
        VCALL(*menu, open);
    }
}

INLINE void menuRender(RenderContext* ctx, Transforms* transforms) {
    if (!menuIsOpen()) {
        return;
    }
    VCALL(*current_menu, render, ctx, transforms);
}
