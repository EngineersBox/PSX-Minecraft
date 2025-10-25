#include "save.h"

#include <stdlib.h>

#include "../../logging/logging.h"

void saveFromRaw(UNUSED void* data) {
    UNIMPLEMENTED();
}

void saveDestroy(Save* save) {
    free(save->name);
}
