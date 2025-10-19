#include "main.h"

#include "../blocks/block.h"
#include "../../render/render_context.h"
#include "../../resources/assets.h"
#include "../../resources/asset_indices.h"
#include "../../structure/cvector_utils.h"
#include "../../util/preprocessor.h"
#include "../../util/interface99_extensions.h"
#include "../../ui/background.h"
#include "../../ui/components/button.h"
#include "../../ui/components/cursor.h"
#include "menu.h"
#include "menu_id.h"
#include "stdlib.h"

static Texture logo = {0};

typedef enum MainMenuButton {
    MAIN_MENU_SINGLEPLAYER = 0,
    MAIN_MENU_MULTIPLAYER = 1,
    MAIN_MENU_MODS_AND_TEXTURE_PACKS = 2,
    MAIN_MENU_OPTIONS = 3,
    MAIN_MENU_QUIT = 4
} MainMenuButton;

#define BUTTONS_POS_X ((SCREEN_XRES >> 1) - (UI_BUTTON_TEXTURE_WIDTH >> 1))
#define BUTTONS_START_POS_Y ((SCREEN_YRES >> 2) + 48)
#define BUTTONS_OFFSET_POS_Y 24

#define BUTTONS_SMALL_WIDTH ((UI_BUTTON_TEXTURE_WIDTH >> 1) - 2)

INLINE UIButton* addButton(UI* ui,
                           const char* text,
                           i16 x,
                           i16 y,
                           i16 width) {
    IUIComponent* component = uiAddComponent(ui);
    UIButton* button = uiButtonNew(
        text,
        x,
        y,
        width,
        1
    );
    DYN_PTR(
        component,
        UIButton,
        IUIComponent,
        button
    );
    return button;
}

IUI* mainMenuNew() {
    // NOTE: Maybe its better to malloc sizeof(IUI) + sizeof(MainMenu)
    //       together and use the offset address for the MainMenu
    //       instance like we do for inhertiance-esque objects
    IUI* iui = (IUI*) malloc(sizeof(IUI));
    assert(iui != NULL);
    MainMenu* main_menu = (MainMenu*) malloc(sizeof(MainMenu));
    assert(main_menu != NULL);
    uiInit(&main_menu->ui);
    DYN_PTR(iui, MainMenu, IUI, main_menu);
    // Singleplayer
    addButton(
        &main_menu->ui, 
        "Singleplayer",
        BUTTONS_POS_X,
        BUTTONS_START_POS_Y + (BUTTONS_OFFSET_POS_Y * 0),
        UI_BUTTON_TEXTURE_WIDTH
    );
    // Multiplayer
    UIButton* multiplayer_button = addButton(
        &main_menu->ui,
        "Multiplayer",
        BUTTONS_POS_X,
        BUTTONS_START_POS_Y + (BUTTONS_OFFSET_POS_Y * 1),
        UI_BUTTON_TEXTURE_WIDTH
    );
    multiplayer_button->state = BUTTON_DISABLED;
    // Mods & texture packs
    UIButton* mods_tp_button = addButton(
        &main_menu->ui,
        "Mods and Texture Packs",
        BUTTONS_POS_X,
        BUTTONS_START_POS_Y + (BUTTONS_OFFSET_POS_Y * 2),
        UI_BUTTON_TEXTURE_WIDTH
    );
    mods_tp_button->state = BUTTON_DISABLED;
    // Options
    addButton(
        &main_menu->ui, 
        "Options...",
        BUTTONS_POS_X,
        BUTTONS_START_POS_Y + (BUTTONS_OFFSET_POS_Y * 2) + 12,
        BUTTONS_SMALL_WIDTH
    );
    // Quit
    addButton(
        &main_menu->ui, 
        "Quit Game",
        (SCREEN_XRES >> 1) + 2,
        BUTTONS_START_POS_Y + (BUTTONS_OFFSET_POS_Y * 2) + 12,
        BUTTONS_SMALL_WIDTH
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
    self->ui.active = true;
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
    VCALL(cursor_component, update);
    const UI* ui = &main_menu->ui;
    if (isButtonPressed(ui, MAIN_MENU_SINGLEPLAYER)) {
        menuOpen(MENUID_SINGLEPLAYER);
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
    uiCursorRender(
        &cursor,
        ctx,
        transforms
    );
}
