#pragma once

#ifndef PSXMC_UI_H
#define PSXMC_UI_H

#include <stdbool.h>
#include <psxgte.h>
#include <interface99.h>
#include <metalang99.h>

#include "../structure/cvector.h"
#include "../render/render_context.h"
#include "../render/transforms.h"
#include "../resources/texture.h"
#include "../core/input/input.h"

#define IUIComponent_IFACE \
    vfunc(void, render, VSelf, RenderContext* ctx, Transforms* transforms) \
    vfuncDefault(void, update, VSelf)

void iuiComponentUpdate(VSelf);
void IUIComponent_update(VSelf);

interface(IUIComponent);
typedef struct UIComponent {
    DVECTOR position;
    DVECTOR dimensions;
} UIComponent;

#define DEFN_UI_COMPONENT(name, ...) typedef struct {\
    UIComponent component; \
    __VA_ARGS__ \
} name

#define IUI_IFACE \
    vfuncDefault(void, open, VSelf) \
    vfuncDefault(void, close, VSelf) \
    vfuncDefault(void, render, VSelf, RenderContext* ctx, Transforms* transforms)

void iuiOpen(VSelf);
void IUI_open(VSelf);

void iuiClose(VSelf);
void IUI_close(VSelf);

void iuiRender(VSelf, RenderContext* ctx, Transforms* transforms);
void IUI_render(VSelf, RenderContext* ctx, Transforms* transforms);

#define IUI_EXTENDS (IInputHandler)
interface(IUI);

typedef struct {
    char* title;
    Texture* texture;
    bool active;
    DVECTOR cursor;
    cvector(IUIComponent) components;
} UI;

#define DEFN_UI(name, ...) typedef struct name { \
    UI ui; \
    __VA_ARGS__ \
} name

void uiInit(UI* ui);
void uiRender(const UI* ui, RenderContext* ctx, Transforms* transforms);

IUIComponent* uiAddComponent(UI* ui);

#endif // PSXMC_UI_H
