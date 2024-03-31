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