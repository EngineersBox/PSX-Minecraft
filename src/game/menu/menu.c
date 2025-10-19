#include "menu.h"

#include "../../core/input/input.h"
#include "menus.h"

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
        VCALL(*current_menu, close);
        inputRemoveLastHandler(&input);
    }
    current_menu = menu;
    if (menuIsOpen()) {
        VCALL_SUPER(*menu, IInputHandler, registerInputHandler, &input, NULL);
        VCALL(*menu, open);
    }
}

INLINE void menuRender(RenderContext* ctx, Transforms* transforms) {
    if (!menuIsOpen()) {
        return;
    }
    VCALL(*current_menu, render, ctx, transforms);
}
