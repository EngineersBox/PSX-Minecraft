#include "main.h"

#include "../blocks/block.h"
#include "../../resources/assets.h"
#include "../../resources/asset_indices.h"
#include "../../structure/cvector_utils.h"
#include "../../util/preprocessor.h"
#include "../../util/interface99_extensions.h"
#include "../../ui/background.h"
#include "../../ui/components/button.h"
#include "menu.h"
#include "menu_id.h"
#include "stdlib.h"

static Texture logo = {0};

typedef enum MainMenuButton {
    MAIN_MENU_CREATE_WORLD = 0,
    MAIN_MENU_OPTIONS = 1,
    MAIN_MENU_QUIT = 2
} MainMenuButton;

IUI* mainMenuNew() {
    IUI* iui = (IUI*) malloc(sizeof(IUI));
    MainMenu* main_menu = (MainMenu*) malloc(sizeof(MainMenu));
    uiInit(&main_menu->ui);
    DYN_PTR(iui, MainMenu, IUI, main_menu);
    IUIComponent* create_world_button_component = uiAddComponent(&main_menu->ui);
    UIButton* create_world_button = uiButtonNew();
    DYN_PTR(
        create_world_button_component,
        UIButton,
        IUIComponent,
        create_world_button
    );
    return iui;
}
void mainMenuDestroy(IUI* menu) {
    MainMenu* main_menu = VCAST_PTR(MainMenu*, menu);
    IUIComponent* component;
    cvector_for_each_in(component, main_menu->ui.components) {
        UIButton* button = VCAST_PTR(UIButton*, component);
        free(button);
    }
    free(main_menu);
    free(menu);
}

void mainMenuOpen(VSelf) ALIAS("MainMenu_open");
void MainMenu_open(VSelf) {
    VSELF(MainMenu);
    assetLoadTextureDirect(
        ASSET_BUNDLE__MENU,
        ASSET_TEXTURE__MENU__LOGO,
        &logo
    );
}

void mainMenuClose(VSelf) ALIAS("MainMenu_close");
void MainMenu_close(UNUSED VSelf) {
}

bool isButtonPressed(const UI* ui, MainMenuButton index) {
    const IUIComponent* component = &ui->components[index];
    const UIButton* button = VCAST_PTR(UIButton*, component);
    return button->state == BUTTON_PRESSED;
}

InputHandlerState mainMenuInputHandler(UNUSED const Input* input, void* ctx) {
    const MainMenu* main_menu = (MainMenu*) ctx;
    const UI* ui = &main_menu->ui;
    if (isButtonPressed(ui, MAIN_MENU_CREATE_WORLD)) {
        menuOpen(MENUID_CREATE_WORLD);
        return INPUT_HANDLER_RELINQUISH;
    } else if (isButtonPressed(ui, MAIN_MENU_OPTIONS)) {
        menuOpen(MENUID_OPTIONS);
        return INPUT_HANDLER_RELINQUISH;
    } else if (isButtonPressed(ui, MAIN_MENU_QUIT)) {
        abort();
        return INPUT_HANDLER_RELINQUISH;
    }
    return INPUT_HANDLER_RETAIN;
}

void mainMenuRegisterInputHandler(VSelf, Input* input, void* ctx) ALIAS("MainMenu_registerInputHandler");
void MainMenu_registerInputHandler(VSelf, Input* input, UNUSED void* ctx) {
    VSELF(MainMenu);
    const InputHandlerVTable handler = (InputHandlerVTable) {
        .ctx = self,
        .input_handler = mainMenuInputHandler
    };
    inputAddHandler(
        input,
        handler
    );
}

void mainMenuRender(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("MainMenu_render");
void MainMenu_render(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(MainMenu);
    const u32* background_ot_object = allocateOrderingTable(ctx, 2);
    backgroundDraw(
        ctx,
        background_ot_object,
        2 * BLOCK_TEXTURE_SIZE,
        0 * BLOCK_TEXTURE_SIZE
    );
    uiRender(&self->ui, ctx, transforms);
}
