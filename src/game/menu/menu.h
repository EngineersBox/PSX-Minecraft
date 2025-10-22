#pragma once

#ifndef _GAME_MENU__MENU_H_
#define _GAME_MENU__MENU_H_

#include "menu_id.h"
#include "../../ui/ui.h"
#include "../../render/render_context.h"

extern IUI* current_menu;

#define menuIsOpen() (current_menu != NULL)

void menuOpen(const EMenuID menu_id);
void menuSetCurrent(IUI* menu);
void menuRender(RenderContext* ctx, Transforms* transforms);

typedef IUI* (*MenuConstructor)();
typedef void (*MenuDestructor)(IUI* menu);

#endif // _GAME_MENU__MENU_H_
