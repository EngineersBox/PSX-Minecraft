#pragma once

#ifndef _GAME_MENU__SINGLEPLAYER_H_
#define _GAME_MENU__SINGLEPLAYER_H_

#include <interface99.h>

#include "../../ui/ui.h"

DEFN_UI(Singleplayer);

ALLOC_CALL(singleplayerDestroy, 1) IUI* singleplayerNew();
void singleplayerDestroy(IUI* menu);

#define Singleplayer_open_CUSTOM ()
void singleplayerOpen(VSelf);
void Singleplayer_open(VSelf);

void singleplayerRegisterInputHandler(VSelf, Input* input, void* ctx);
void Singleplayer_registerInputHandler(VSelf, Input* input, void* ctx);

#define Singleplayer_render_CUSTOM ()
void singleplayerRender(VSelf, RenderContext* ctx, Transforms* transforms);
void Singleplayer_render(VSelf, RenderContext* ctx, Transforms* transforms);

impl(IInputHandler, Singleplayer);
impl(IUI, Singleplayer);

#endif // _GAME_MENU__SINGLEPLAYER_H_