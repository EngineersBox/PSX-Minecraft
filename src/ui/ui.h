#pragma once

#ifndef PSX_MINECRAFT_UI_H
#define PSX_MINECRAFT_UI_H

#include <stdbool.h>
#include <psxgte.h>
#include <interface99.h>

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

#define DEFN_UI_COMPONENT(name, ...) \
    typedef struct {\
        UIComponent component; \
        __VA_ARGS__ \
    } name;

typedef struct {
    char* title;
    bool active;
    cvector(IUIComponent) components;
} UI;

// typedef struct {
//     char* title;
//     bool active;
//     bool open;
// } UI;
//
// #define DEFN_COMPONENTS(...) struct { \
//         UIComponent __VA_ARGS__ \
//     } components
//
// #define DEFN_UI(name, components) struct { \
//         UI ui; \
//         components \
//     } name
//
// DEFN_UI(
//     test_ui,
//     DEFN_COMPONENTS(
//         test, test2;
//     );
// );

#endif // PSX_MINECRAFT_UI_H
