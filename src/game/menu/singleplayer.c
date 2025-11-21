#include "singleplayer.h"

#include <psxgpu.h>

#include "menu.h"
#include "menu_id.h"
#include "../blocks/block.h"
#include "../../render/render_context.h"
#include "../../render/font.h"
#include "../../resources/assets.h"
#include "../../resources/asset_indices.h"
#include "../../resources/texture.h"
#include "../../structure/cvector_utils.h"
#include "../../structure/primitive/primitive.h"
#include "../../util/preprocessor.h"
#include "../../util/interface99_extensions.h"
#include "../../ui/components/background.h"
#include "../../ui/components/button.h"
#include "../../ui/components/cursor.h"


IUI* singleplayerNew() {
    IUI* iui = (IUI*) malloc(sizeof(IUI));
    assert(iui != NULL);
    Singleplayer* menu = (Singleplayer*) malloc(sizeof(Singleplayer));
    assert(menu != NULL);
    uiInit(&menu->ui);
    DYN_PTR(iui, Singleplayer, IUI, menu);
    UI* ui = &menu->ui;
    IUIComponent* upper_bg_1_background_component = uiAddComponent(ui);
    UIBackground* upper_bg_1_background = uiBackgroundNew(
        &textures[ASSET_TEXTURE__STATIC__TERRAIN],
        vec2_i16(0, 0),
        vec2_i16(160, 16),
        vec2_i16(32, 0),
        vec2_i16(16, 16),
        vec3_rgb(63, 63, 63),
        1
    );
    DYN_PTR(
        upper_bg_1_background_component,
        UIBackground,
        IUIComponent,
        upper_bg_1_background
    );
    IUIComponent* upper_bg_2_background_component = uiAddComponent(ui);
    UIBackground* upper_bg_2_background = uiBackgroundNew(
        &textures[ASSET_TEXTURE__STATIC__TERRAIN],
        vec2_i16(160, 0),
        vec2_i16(160, 16),
        vec2_i16(32, 0),
        vec2_i16(16, 16),
        vec3_rgb(63, 63, 63),
        1
    );
    DYN_PTR(
        upper_bg_2_background_component,
        UIBackground,
        IUIComponent,
        upper_bg_2_background
    );
    IUIComponent* select_button_component = uiAddComponent(ui);
    UIButton* select_button = uiButtonNew(
        "select",
        vec2_i16(6, 188),
        150,
        1
    );
    DYN_PTR(
        select_button_component,
        UIButton,
        IUIComponent,
        select_button
    );
    IUIComponent* rename_button_component = uiAddComponent(ui);
    UIButton* rename_button = uiButtonNew(
        "rename",
        vec2_i16(6, 212),
        70,
        1
    );
    DYN_PTR(
        rename_button_component,
        UIButton,
        IUIComponent,
        rename_button
    );
    IUIComponent* delete_button_component = uiAddComponent(ui);
    UIButton* delete_button = uiButtonNew(
        "delete",
        vec2_i16(86, 212),
        70,
        1
    );
    DYN_PTR(
        delete_button_component,
        UIButton,
        IUIComponent,
        delete_button
    );
    IUIComponent* create_button_component = uiAddComponent(ui);
    UIButton* create_button = uiButtonNew(
        "create",
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
        "cancel",
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
    IUIComponent* bottom_bg_1_background_component = uiAddComponent(ui);
    UIBackground* bottom_bg_1_background = uiBackgroundNew(
        &textures[ASSET_TEXTURE__STATIC__TERRAIN],
        vec2_i16(0, 180),
        vec2_i16(160, 60),
        vec2_i16(32, 0),
        vec2_i16(16, 16),
        vec3_rgb(63, 63, 63),
        1
    );
    DYN_PTR(
        bottom_bg_1_background_component,
        UIBackground,
        IUIComponent,
        bottom_bg_1_background
    );
    IUIComponent* bottom_bg_2_background_component = uiAddComponent(ui);
    UIBackground* bottom_bg_2_background = uiBackgroundNew(
        &textures[ASSET_TEXTURE__STATIC__TERRAIN],
        vec2_i16(160, 180),
        vec2_i16(160, 60),
        vec2_i16(32, 0),
        vec2_i16(16, 16),
        vec3_rgb(63, 63, 63),
        1
    );
    DYN_PTR(
        bottom_bg_2_background_component,
        UIBackground,
        IUIComponent,
        bottom_bg_2_background
    );
    return iui;
}

void singleplayerDestroy(IUI* menu) {
    Singleplayer* self = VCAST_PTR(Singleplayer*, menu);
    IUIComponent* component;
    cvector_for_each_in(component, self->ui.components) {
        void* component_instance = VCAST_PTR(void*, component);
        free(component_instance);
    }
    free(self);
    free(menu);
}

void singleplayerOpen(VSelf) ALIAS("Singleplayer_open");
void Singleplayer_open(VSelf) {
    VSELF(Singleplayer);
    self->ui.active = true;
}

static InputHandlerState singleplayerInputHandler(UNUSED const Input* input, void* ctx) {
    VCALL(cursor_component, update);
    const Singleplayer* menu = (Singleplayer*) ctx;
    const UI* ui = &menu->ui;
    IUIComponent* component;
    cvector_for_each_in(component, ui->components) {
        VCALL(*component, update);
    }
    //  TODO: Finish implementing logic for components being interacted with
    return INPUT_HANDLER_RELINQUISH;
}

void singleplayerRegisterInputHandler(VSelf, Input* input, void* ctx) ALIAS("Singleplayer_registerInputHandler");
void Singleplayer_registerInputHandler(VSelf, Input* input, UNUSED void* ctx) {
    VSELF(Singleplayer);
    const InputHandlerVTable handler = (InputHandlerVTable) {
        .ctx = self,
        .input_handler = singleplayerInputHandler
    };
    inputAddHandler(
        input,
        handler
    );
}

void singleplayerRender(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("Singleplayer_render");
void Singleplayer_render(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(Singleplayer);
    uiRender(&self->ui, ctx, transforms);
    renderClearConstraintsIndex(ctx, 1);
    uiCursorRender(
        &cursor,
        ctx,
        transforms
    );
}
