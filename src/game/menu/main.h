#pragma once

#ifndef _GAME_MENU__MAIN_H_
#define _GAME_MENU__MAIN_H_

#include <interface99.h>

#include "../../ui/ui.h"

DEFN_UI(MainMenu);

ALLOC_CALL(mainMenuDestroy, 1) IUI* mainMenuNew();
void mainMenuDestroy(IUI* menu);

#define MainMenu_open_CUSTOM ()
void mainMenuOpen(VSelf);
void MainMenu_open(VSelf);

void mainMenuRegisterInputHandler(VSelf, Input* input, void* ctx);
void MainMenu_registerInputHandler(VSelf, Input* input, void* ctx);

#define MainMenu_render_CUSTOM ()
void mainMenuRender(VSelf, RenderContext* ctx, Transforms* transforms);
void MainMenu_render(VSelf, RenderContext* ctx, Transforms* transforms);

impl(IInputHandler, MainMenu);
impl(IUI, MainMenu);

#endif // _GAME_MENU__MAIN_H_
