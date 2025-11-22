#include "ui.h"

#include "../structure/cvector_utils.h"

void uiInit(UI* ui) {
    ui->active = false;
    ui->cursor = (DVECTOR) {0};
    ui->texture = NULL;
    ui->components = NULL;
    cvector_init(ui->components, 0, NULL);
}

void iuiOpen(VSelf) ALIAS("IUI_open");
void IUI_open(UNUSED VSelf) {
    VSELF(UI);
    self->active = true;
}

void iuiClose(VSelf) ALIAS("IUI_close");
void IUI_close(UNUSED VSelf) {
    VSELF(UI);
    self->active = false;
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

void iuiRender(VSelf, RenderContext* ctx, Transforms* transforms) ALIAS("IUI_render");
INLINE void IUI_render(VSelf, RenderContext* ctx, Transforms* transforms) {
    VSELF(UI);
    uiRender(self, ctx, transforms);
}

IUIComponent* uiAddComponent(UI* ui) {
    cvector_push_back(ui->components, (IUIComponent) {0});
    return &ui->components[cvector_size(ui->components) - 1];
}

void iuiComponentUpdate(VSelf) ALIAS("IUIComponent_update");
void IUIComponent_update(UNUSED VSelf) {
    // Do nothing
}
