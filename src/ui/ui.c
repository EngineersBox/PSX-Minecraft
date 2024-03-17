#include "ui.h"

#include "../structure/cvector_utils.h"

void uiRender(UI* ui, RenderContext* ctx, Transforms* transforms) {
    if (ui->components == NULL) {
        return;
    }
    IUIComponent* component;
    cvector_for_each_in(component, ui->components) {
        VCALL(*component, render, ctx, transforms);
    }
}