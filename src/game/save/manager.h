#pragma once

#ifndef _GAME_SAVE__MANAGER_H_
#define _GAME_SAVE__MANAGER_H_

#include <stdbool.h>

#include "save.h"
#include "../world/world.h"
#include "../world/world_structure.h"
#include "../../structure/cvector.h"

typedef struct SaveManager {
    bool card_0_present: 1;
    bool card_1_present: 1;
    u8 _pad: 6;
    cvector(Save) saves;
} SaveManager;

void saveManagerInit(SaveManager* save_manager);
void saveManagerDestroy(SaveManager* save_manager);

void saveManagerUpdate(SaveManager* save_manager);

void saveManagerSaveWorld(SaveManager* save_manager, World* world);
World* saveManagerLoadWorld(SaveManager* save_manager, Save* save);

#endif // _GAME_SAVE__MANAGER_H_
