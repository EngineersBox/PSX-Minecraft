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
//       the texture is unloaded.

typedef struct {
    char* title;
    bool active;
    cvector(IUIComponent) components;
} UI;

void uiRender(const UI* ui, RenderContext* ctx, Transforms* transforms);

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
