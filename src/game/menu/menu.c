#include "menu.h"

#include <interface99.h>

#include "menus.h"
#include "../../core/input/input.h"
// #include "../../logging/logging.h"
#include "../../ui/components/button.h"
#include "../../util/interface99_extensions.h"

IUI current_menu = {0};
EMenuID current_menu_id = 0;

void menuClose() {
    if (!menuIsOpen()) {
        return;
    }
    // Menus act as a single element stack, so
    // the last input handler will alawys be
    // registered to the currently opened menu.
    // This cirumstance occurs between menu
    // transitions
    // DEBUG_LOG("Closing current menu\n");
    VCALL(current_menu, close);
    current_menu = (IUI) {0};
    // DEBUG_LOG("Removing last handler\n");
    inputPopHandler(&input);
    inputSetFocusedHandler(&input, NULL);
    // DEBUG_LOG("Destroying current menu\n");
    const MenuDestructor dtor = menu_destructors[current_menu_id];
    dtor(&current_menu);
}

static void menuSetCurrent(const IUI menu, const EMenuID menu_id) {
    menuClose();
    // DEBUG_LOG("Setting current menu to new menu\n");
    current_menu = menu;
    current_menu_id = menu_id;
    if (menuIsOpen()) {
        // DEBUG_LOG("Registering new menu input handler\n");
        VCALL_SUPER(menu, IInputHandler, registerInputHandler, &input, NULL);
        // DEBUG_LOG("Opening new menu\n");
        VCALL(menu, open);
        // DEBUG_LOG("Opened menu\n");
    }
}

void menuOpen(const EMenuID menu_id) {
    const MenuConstructor ctor = menu_constructors[menu_id];
    const IUI next_menu = ctor();
    menuSetCurrent(next_menu, menu_id);
}

INLINE void menuRender(RenderContext* ctx, Transforms* transforms) {
    if (!menuIsOpen()) {
        return;
    }
    VCALL(current_menu, render, ctx, transforms);
}

bool isButtonPressed(UI* ui, size_t index) {
    IUIComponent* component = &ui->components[index];
    UIButton* button = VCAST_PTR(UIButton*, component);
    return button->state == BUTTON_PRESSED;
}
