#pragma once

#ifndef PSX_MINECRAFT_UI_H
#define PSX_MINECRAFT_UI_H

#include "../render/render_context.h"
#include "../render/transforms.h"

typedef struct UIComponent UIComponent;

typedef void (*UIComponentAction)(UIComponent* component);
typedef void (*UIComponentRender)(UIComponent* component, RenderContext* ctx, Transforms* transforms);

typedef struct UIComponent {
    struct {
        int32_t x;
        int32_t y;
    } position;
    struct {
        int32_t width;
        int32_t height;
    } dimensions;
    void* state;
    // Instance methods
    UIComponentAction action;
    UIComponentRender render;
} UIComponent;

typedef struct {
    UIComponent* components;
} UI;

#endif // PSX_MINECRAFT_UI_H
