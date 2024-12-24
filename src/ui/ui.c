#include "ui.h"

#include "../structure/cvector_utils.h"

void uiInit(UI* ui) {
    ui->active = false;
    ui->cursor = (DVECTOR) {0};
    ui->texture = NULL;
    ui->components = NULL;
    cvector_init(ui->components, 0, NULL);
}

void uiRender(const UI* ui, RenderContext* ctx, Transforms* transforms) {
    if (ui->components == NULL || !ui->active) {
        return;
    }
    IUIComponent* component;
    cvector_for_each_in(component, ui->components) {
        VCALL(*component, render, ctx, transforms);
    }
}

IUIComponent* uiAddComponent(UI* ui) {
    cvector_push_back(ui->components, (IUIComponent) {});
    return &ui->components[cvector_size(ui->components) - 1];
}

void iuiComponentCursorAction(VSelf, const DVECTOR* cursor_position, u8 press_state) ALIAS("IUIComponent_cursorAction");
void IUIComponent_cursorAction(VSelf, const DVECTOR* cursor_position, u8 press_state) {
    // Do nothing
}

void iuiComponentUpdate(VSelf) ALIAS("IUIComponent_update");
void IUIComponent_update(VSelf) {
    // Do nothing
}
