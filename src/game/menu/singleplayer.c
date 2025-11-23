#include "singleplayer.h"

#include <psxgpu.h>

#include "menu.h"
#include "menu_id.h"
#include "../blocks/block.h"
#include "../../logging/logging.h"
#include "../../render/render_context.h"
#include "../../render/font.h"
#include "../../resources/assets.h"
#include "../../resources/asset_indices.h"
#include "../../resources/texture.h"
#include "../../structure/cvector_utils.h"
#include "../../structure/primitive/primitive.h"
#include "../../util/preprocessor.h"
#include "../../util/interface99_extensions.h"
#include "../../ui/background.h"
#include "../../ui/components/background.h"
#include "../../ui/components/button.h"
#include "../../ui/components/cursor.h"

typedef enum SingleplayerMenuButton {
    // Values are indices into UI components array
    SINGLEPLAYER_MENU_PLAY = 2,
    SINGLEPLAYER_MENU_RENAME = 3,
    SINGLEPLAYER_MENU_DELETE = 4,
    SINGLEPLAYER_MENU_CREATE = 5,
    SINGLEPLAYER_MENU_CANCEL = 7
} SingleplayerMenuButton;

IUI* singleplayerMenuNew() {
    IUI* iui = (IUI*) malloc(sizeof(IUI));
    assert(iui != NULL);
    SingleplayerMenu* menu = (SingleplayerMenu*) malloc(sizeof(SingleplayerMenu));
    assert(menu != NULL);
    uiInit(&menu->ui);
    DYN_PTR(iui, SingleplayerMenu, IUI, menu);
    UI* ui = &menu->ui;
    IUIComponent* middle_bg_1_background_component = uiAddComponent(ui);
    UIBackground* middle_bg_1_background = uiBackgroundNew(
        &textures[ASSET_TEXTURE__STATIC__TERRAIN],
        vec2_i16(0, 16),
        vec2_i16(160, 164),
        vec2_i16(32, 0),
        vec2_i16(16, 16),
        vec3_rgb(0x14, 0x14, 0x14),
        1
    );
    DYN_PTR(
        middle_bg_1_background_component,
        UIBackground,
        IUIComponent,
        middle_bg_1_background
    );
    IUIComponent* middle_bg_2_background_component = uiAddComponent(ui);
    UIBackground* middle_bg_2_background = uiBackgroundNew(
        &textures[ASSET_TEXTURE__STATIC__TERRAIN],
        vec2_i16(160, 16),
        vec2_i16(160, 164),
        vec2_i16(32, 0),
        vec2_i16(16, 16),
        vec3_rgb(0x14, 0x14, 0x14),
        1
    );
    DYN_PTR(
        middle_bg_2_background_component,
        UIBackground,
        IUIComponent,
        middle_bg_2_background
    );
    IUIComponent* play_button_component = uiAddComponent(ui);
    UIButton* play_button = uiButtonNew(
        "Play World",
        vec2_i16(6, 188),
        150,
        1
    );
    play_button->state = BUTTON_DISABLED;
    DYN_PTR(
        play_button_component,
        UIButton,
        IUIComponent,
        play_button
    );
    IUIComponent* rename_button_component = uiAddComponent(ui);
    UIButton* rename_button = uiButtonNew(
        "Rename",
        vec2_i16(6, 212),
        70,
        1
    );
    play_button->state = BUTTON_DISABLED;
    DYN_PTR(
        rename_button_component,
        UIButton,
        IUIComponent,
        rename_button
    );
    IUIComponent* delete_button_component = uiAddComponent(ui);
    UIButton* delete_button = uiButtonNew(
        "Delete",
        vec2_i16(86, 212),
        70,
        1
    );
    play_button->state = BUTTON_DISABLED;
    DYN_PTR(
        delete_button_component,
        UIButton,
        IUIComponent,
        delete_button
    );
    IUIComponent* create_button_component = uiAddComponent(ui);
    UIButton* create_button = uiButtonNew(
        "Create New World",
        vec2_i16(164, 188),
        150,
        1
    );
    DYN_PTR(
        create_button_component,
        UIButton,
        IUIComponent,
        create_button
    );
    IUIComponent* cancel_button_component = uiAddComponent(ui);
    UIButton* cancel_button = uiButtonNew(
        "Cancel",
        vec2_i16(164, 212),
        150,
        1
    );
    DYN_PTR(
        cancel_button_component,
        UIButton,
        IUIComponent,
        cancel_button
    );
    return iui;
}

void singleplayerMenuDestroy(IUI* menu) {
    SingleplayerMenu* self = VCAST_PTR(SingleplayerMenu*, menu);
    IUIComponent* component;
    cvector_for_each_in(component, self->ui.components) {
        void* component_instance = VCAST_PTR(void*, component);
        free(component_instance);
    }
    free(self);
    free(menu);
}

static InputHandlerState singleplayerMenuInputHandler(UNUSED const Input* input, void* ctx) {
    VCALL(cursor_component, update);
    SingleplayerMenu* menu = (SingleplayerMenu*) ctx;
    UI* ui = &menu->ui;
    IUIComponent* component;
    cvector_for_each_in(component, ui->components) {
        VCALL(*component, update);
    }
    if (isButtonPressed(ui, SINGLEPLAYER_MENU_PLAY)) {
        // TODO: Load world
        return INPUT_HANDLER_RELEASE;
    } else if (isButtonPressed(ui, SINGLEPLAYER_MENU_RENAME)) {
        menuOpen(MENUID_WORLD_RENAME);
        return INPUT_HANDLER_RELEASE;
    } else if (isButtonPressed(ui, SINGLEPLAYER_MENU_DELETE)) {
        menuOpen(MENUID_WORLD_DELETE);
        return INPUT_HANDLER_RELEASE;
    } else if (isButtonPressed(ui, SINGLEPLAYER_MENU_CREATE)) {
        menuOpen(MENUID_WORLD_CREATE);
        return INPUT_HANDLER_RELEASE;
    } else if (isButtonPressed(ui, SINGLEPLAYER_MENU_CANCEL)) {
        menuOpen(MENUID_MAIN);
        return INPUT_HANDLER_RELEASE;
    }
    return INPUT_HANDLER_RETAIN;
}

void singleplayerMenuRegisterInputHandler(VSelf, Input* input, void* ctx) ALIAS("SingleplayerMenu_registerInputHandler");
void SingleplayerMenu_registerInputHandler(VSelf, Input* input, UNUSED void* ctx) {
    VSELF(SingleplayerMenu);
    const InputHandlerVTable handler = (InputHandlerVTable) {
        .ctx = self,
        .input_handler = singleplayerMenuInputHandler
    };
    inputAddHandler(
        input,
        handler
    );
}

void singleplayerMenuRender(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("SingleplayerMenu_render");
void SingleplayerMenu_render(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(SingleplayerMenu);
    u32* background_ot_object = allocateOrderingTable(ctx, 2);
    backgroundDraw(
        ctx,
        background_ot_object,
        2 * BLOCK_TEXTURE_SIZE,
        0 * BLOCK_TEXTURE_SIZE
    );
    uiRender(&self->ui, ctx, transforms);
    fontPrintCentreOffset(
        ctx,
        SCREEN_XRES >> 1,
        BLOCK_TEXTURE_SIZE - FONT_CHARACTER_SPRITE_HEIGHT - 3,
        0,
        1,
        "Select World"
    );
    renderClearConstraintsIndex(ctx, 1);
    uiCursorRender(
        &cursor,
        ctx,
        transforms
    );
}
