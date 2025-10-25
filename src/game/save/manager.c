#include "manager.h"

#include "../../util/preprocessor.h"

void saveManagerInit(SaveManager* save_manager) {
    assert(save_manager != NULL);
    save_manager->card_0_present = false;
    save_manager->card_1_present = false;
    cvector_init(save_manager->saves, 0, NULL);
}

INLINE void saveManagerDestroy(SaveManager* save_manager) {
    cvector_free(save_manager->saves);
}

void saveManagerUpdate(SaveManager* save_manager) {
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

void saveManagerSaveWorld(SaveManager* save_manager, World* world) {
    UNIMPLEMENTED();
}

World* saveManagerLoadWorld(SaveManager* save_manager, Save* save) {
    UNIMPLEMENTED();
    return NULL;
}
