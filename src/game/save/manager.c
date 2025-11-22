#include "manager.h"

#include "../../logging/logging.h"
#include "save.h"

void saveManagerInit(SaveManager* save_manager) {
    assert(save_manager != NULL);
    save_manager->card_0_present = false;
    save_manager->card_1_present = false;
    cvector_init(save_manager->saves, 0, (void(*)(void*)) saveDestroy);
}

INLINE void saveManagerDestroy(SaveManager* save_manager) {
    cvector_free(save_manager->saves);
}

void saveManagerUpdate(UNUSED SaveManager* save_manager) {
    UNIMPLEMENTED();
    // TODO: 1. Poll IO to detect memory card(s) present
    //       2. If present and no saves loaded:
    //         a. Check segments for Save headers
    //         b. Load Saves from headers
    //         c. Done
    //       3. If present and saves loaded:
    //         b. Done
    //       3. If not present and saves loaded:
    //         a. Set flag for saving not possible
    //         b, Done
    //       4. Done
}

void saveManagerSaveWorld(UNUSED SaveManager* save_manager, UNUSED World* world) {
    UNIMPLEMENTED();
}

World* saveManagerLoadWorld(UNUSED SaveManager* save_manager, UNUSED Save* save) {
    UNIMPLEMENTED();
    return NULL;
}
