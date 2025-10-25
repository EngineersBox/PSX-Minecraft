#pragma once

#ifndef _GAME_MENU__SINGLEPLAYER_H_
#define _GAME_MENU__SINGLEPLAYER_H_

#include <interface99.h>

#include "../../ui/ui.h"

DEFN_UI(SingleplayerMenu);

IUI* singleplayerMenuNew();
void singleplayerMenuDestroy(IUI* menu);

void singleplayerMenuRegisterInputHandler(VSelf, Input* input, void* ctx);
void SingleplayerMenu_registerInputHandler(VSelf, Input* input, void* ctx);

#define SingleplayerMenu_render_CUSTOM ()
void singleplayerMenuRender(VSelf, RenderContext* ctx, Transforms* transforms);
void SingleplayerMenu_render(VSelf, RenderContext* ctx, Transforms* transforms);

impl(IInputHandler, SingleplayerMenu);
impl(IUI, SingleplayerMenu);

#endif // _GAME_MENU__SINGLEPLAYER_H_
