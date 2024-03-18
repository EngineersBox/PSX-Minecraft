#include "ui.h"

#include "../structure/cvector_utils.h"

void uiInit(UI* ui) {
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