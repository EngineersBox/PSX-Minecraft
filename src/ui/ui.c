#include "ui.h"

#include "../structure/cvector_utils.h"

void uiRender(const UI* ui, RenderContext* ctx, Transforms* transforms) {
    if (ui->components == NULL || !ui->active) {
        return;
    }
    IUIComponent* component;
    cvector_for_each_in(component, ui->components) {
        VCALL(*component, render, ctx, transforms);
    }
}