#include "menu.h"

#include <interface99.h>

#include "menus.h"
#include "../../core/input/input.h"
#include "../../logging/logging.h"
#include "../../math/math_utils.h"
#include "../../ui/components/button.h"
#include "../../ui/components/cursor.h"
#include "../../util/interface99_extensions.h"

IUI* current_menu = NULL;
Timestamp menu_debounce = 0;

void menuOpen(const EMenuID menu_id) {
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

bool isButtonPressed(UI* ui, size_t index) {
    IUIComponent* component = &ui->components[index];
    UIButton* button = VCAST_PTR(UIButton*, component);
    if (button->state == BUTTON_DISABLED) {
        return false;
    }
    button->state = quadIntersectLiteral(
        &cursor.component.position,
        button->component.position.vx,
        button->component.position.vy,
        button->component.dimensions.vx,
        button->component.dimensions.vy
    );
    return button->state == BUTTON_PRESSED;
}
