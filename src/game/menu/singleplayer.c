#include "singleplayer.h"

#include <psxgpu.h>

#include "menu.h"
#include "menu_id.h"
#include "../blocks/block.h"
#include "../../render/render_context.h"
#include "../../render/font.h"
#include "../../resources/assets.h"
#include "../../resources/asset_indices.h"
#include "../../structure/cvector_utils.h"
#include "../../structure/primitive/primitive.h"
#include "../../util/preprocessor.h"
#include "../../util/interface99_extensions.h"
#include "../../ui/background.h"
#include "../../ui/components/button.h"
#include "../../ui/components/cursor.h"

typedef enum SingleplayerMenuButton {
    SINGLEPLAYER_MENU_PLAY_WORLD = 0,
    SINGLEPLAYER_MENU_RENAME_WORLD = 1,
    SINGLEPLAYER_MENU_DELETE_WORLD = 2,
    SINGLEPLAYER_MENU_CREATE_WORLD = 3,
    SINGLEPLAYER_MENU_CANCEL = 4
} SingleplayerMenuButton;

IUI* singleplayerMenuNew() {

}

void singleplayerMenuDestroy(IUI* menu) {
    SingleplayerMenu* singleplayer_menu = VCAST_PTR(SingleplayerMenu*, menu);
    IUIComponent* component;
    cvector_for_each_in(component, singleplayer_menu->ui.components) {
        free(component->self);
    }
    free(singleplayer_menu);
    free(menu);
}

void singleplayerMenuRegisterInputHandler(VSelf, Input* input, void* ctx) ALIAS("SingleplayerMenu_registerInputHandler");
void SingleplayerMenu_registerInputHandler(VSelf, Input* input, void* ctx) {
    VSELF(SingleplayerMenu);
}

void singleplayerMenuRender(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("SingleplayerMenu_render");
void SingleplayerMenu_render(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(SingleplayerMenu);
}
