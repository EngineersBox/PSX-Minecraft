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

// TODO: Texture for UI will be loaded in a fixed position in the VRAM
//       buffer, since only one UI view can be active at any given time.
//       When the UI is opened, the texture is loaded and when it is closed,
//       the texture is unloaded or just ignored until next opening overwriting
//       it in place.

#define IUI_IFACE \
    vfunc(void, open, VSelf) \
    vfunc(void, close, VSelf)

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

// #define _TYPE_AS_UI_COMPONENT_PTR(x) IUIComponent* x
//
// #define DEFN_COMPONENTS(...) struct { \
//         ML99_EVAL(ML99_variadicsForEach( \
//             ML99_compose(v(ML99_semicoloned), ML99_reify(v(_TYPE_AS_UI_COMPONENT_PTR))), \
//             v(__VA_ARGS__) \
//         )) \
//     } components
//
// #define DEFN_UI(name, components) struct { \
//         UI ui; \
//         components \
//     } name
//
// #define TYPE_DEFN_UI(name, components) typedef DEFN_UI(name, components)

// Example
// TYPE_DEFN_UI(
//     test_ui,
//     DEFN_COMPONENTS(
//         test, test2
//     );
// );

#endif // PSX_MINECRAFT_UI_H
