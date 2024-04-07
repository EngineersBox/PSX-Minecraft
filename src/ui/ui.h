#pragma once

#ifndef PSX_MINECRAFT_UI_H
#define PSX_MINECRAFT_UI_H

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
    vfunc(void, action, VSelf, const DVECTOR* cursor_position, const bool pressed) \
    vfunc(void, render, VSelf, RenderContext* ctx, Transforms* transforms)

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
    vfunc(void, open, VSelf) \
    vfunc(void, close, VSelf)

#define IUI_EXTENDS (IInputHandler)
interface(IUI);

typedef struct {
    char* title;
    Texture* texture;
    bool active;
    DVECTOR cursor;
    cvector(IUIComponent) components;
} UI;

#define DEFN_UI(name, ...) typedef struct { \
    UI ui; \
    __VA_ARGS__ \
} name

void uiInit(UI* ui);
void uiRender(const UI* ui, RenderContext* ctx, Transforms* transforms);

IUIComponent* uiAddComponent(UI* ui);

#endif // PSX_MINECRAFT_UI_H
