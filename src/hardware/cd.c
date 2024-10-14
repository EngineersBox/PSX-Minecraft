#include "cd.h"

#include <stdlib.h>

void* cdReadDataSync(const char* filename, CdlModeFlag mode) {
    CdlFILE file;
    if (!CdSearchFile(&file, filename)) {
        printf("Unable to find %s\n", filename);
        return NULL;
    }
    return cdReadFileSync(&file, mode);
}

void* cdReadFileSync(const CdlFILE* file, CdlModeFlag mode) {
    int read_size;
    switch (mode) {
        case CdlModeSpeed:
            read_size = 2048;
            break;
        case CdlModeSize:
            read_size = 2340;
            break;
        default:
            printf("[CD] Unimplemented read mode: %d\n", mode);
            return NULL;
    }
    const int sector_count = (file->size / read_size) + 1;
    int result = CdControlB(CdlSetloc, &file->pos, NULL);
    if (result == 0) {
        printf("[CD] Previous pending command not finished\n");
        return NULL;
    } else if (result == -1) {
        printf("[CD] Missing required parameter for CdControlB\n");
        return NULL;
    }
    u8* data = (u8*) malloc(sector_count * read_size);
    if (!CdRead(sector_count, (void*) data, CdlModeSpeed)) {
        printf("[CD] Failed to read %s file from CD\n", file->name);
        return NULL;
    }
    u8 res_buf = 0;
    result = CdReadSync(0, &res_buf);
    if (result == -1) {
        printf("[CD] Read sync failed: %d\n", res_buf);
        return NULL;
    } else if (result == -2) {
        printf("[CD] Read aborted\n");
        return NULL;
    }
    return data;
}

