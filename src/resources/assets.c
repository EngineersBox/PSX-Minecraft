#include "assets.h"

#include <lzp/lzp.h>
#include <lzp/lzqlp.h>
#include <smd/smd.h>
#include <psxgpu.h>
#include <psxcd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../logging/logging.h"

typedef void (*AssetLoader)(int lzp_index);

typedef void (*AssetFreer)(void);

typedef struct {
    AssetLoader loader;
    AssetFreer freer;
    char* name;
} AssetBundle;

inline void _loadTextures(int lzp_index);

inline void _freeTextures();

inline void _loadModels(int lzp_index);

inline void _freeModels();

AssetBundle ASSET_BUNDLES[] = {
    (AssetBundle){
        .loader = _loadTextures,
        .freer = _freeTextures,
        .name = "textures"
    },
    (AssetBundle){
        .loader = _loadModels,
        .freer = _freeModels,
        .name = "models"
    },
    (AssetBundle) { NULL, NULL, NULL }
};

Texture* textures;
bool assets_loaded = false;
u8* _lz_resources = NULL;

void _loadTextures(const int lzp_index) {
    TIM_IMAGE tim = {};
    QLP_HEAD* tex_buff = (QLP_HEAD*) malloc(lzpFileSize(lz_resources, lzp_index));
    lzpUnpackFile(tex_buff, lz_resources, lzp_index);
    const int file_count = qlpFileCount(tex_buff);
    DEBUG_LOG("[TEXTURE] Loading %d texture(s)\n", file_count);
    if (file_count > 0) {
        textures = (Texture*) calloc(file_count, sizeof(Texture));
    }
    for (int i = 0; i < file_count; i++) {
        const QLP_FILE* file = qlpFileEntry(i, tex_buff);
        if (!GetTimInfo((uint32_t*) qlpFileAddr(i, tex_buff), &tim)) {
            assetLoadImage(&tim, &textures[i]);
            DEBUG_LOG(
                "[TEXTURE] Loading: [Name: %s] [Position: (%d,%d)] [Addr: %p] [Mode: 0x%x] [TPage: %d] [CLUT: %d]\n",
                file->name,
                tim.prect->x,
                tim.prect->y,
                tim.caddr,
                tim.mode,
                textures[i].tpage,
                textures[i].clut
            );
        }
    }
    free(tex_buff);
}

void assetLoadImage(const TIM_IMAGE* tim, Texture* texture) {
    LoadImage(tim->prect, tim->paddr);
    if (tim->mode & 0x8) {
        LoadImage(tim->crect, tim->caddr);
    }
    texture->tpage = getTPage(
        tim->mode,
        1,
        tim->prect->x,
        tim->prect->y
    );
    texture->clut = getClut(
        tim->crect->x,
        tim->crect->y
    );
}

void _freeTextures() {
    free(textures);
}

void _loadModels(int lzp_index) {
}

void assetLoadModel(const SMD* smd) {
}

void _freeModels() {
}

void assetsLoad() {
    CdlFILE file;
    if (!CdSearchFile(&file, "\\assets.lzp")) {
        errorAbort("[ASSETS] Unable to find assets.lzp\n");
        return;
    }
    /*int sector = CdPosToInt(&file.pos);*/
    int sector_count = (file.size / 2340) + 1;
    int result = CdControlB(CdlSetloc, &file.pos, NULL);
    if (result == 0) {
        errorAbort("[ASSETS] Previous pending command not finished\n");
        return;
    } else if (result == -1) {
        errorAbort("[ASSETS] Missing required parameter for CdControlB\n");
        return;
    }
    _lz_resources = malloc(sector_count * 2340);
    if (!CdRead(sector_count, (void*) _lz_resources, CdlModeSpeed)) {
        errorAbort("[ASSETS] Failed to read assets.lzp file from CD\n");
        return;
    }
    u8 res_buf = 0;
    result = CdReadSync(0, &res_buf);
    if (result == -1) {
        errorAbort("[ASSETS] CD read failed: %d\n", res_buf);
        return;
    } else if (result == -2) {
        errorAbort("[ASSETS] CD read aborted\n");
        return;
    }
    for (const AssetBundle* bundle = &ASSET_BUNDLES[0]; bundle->name != NULL; bundle++) {
        const int lzp_index = lzpSearchFile(bundle->name, lz_resources);
        if (lzp_index < 0) {
            printf("Asset bundle '%s' not found, skipping\n", bundle->name);
            continue;
        }
        printf("Found asset bundle '%s', loading\n", bundle->name);
        bundle->loader(lzp_index);
    }
    assets_loaded = true;
}

void assetsFree() {
    for (const AssetBundle* bundle = &ASSET_BUNDLES[0]; bundle->name != NULL; bundle++) {
        bundle->freer();
    }
    assets_loaded = false;
}

int assetLoadTextureDirect(const char* bundle, const char* filename, Texture* texture) {
    const int lzp_index = lzpSearchFile(bundle, lz_resources);
    if (lzp_index < 0) {
        printf("[ERROR] No such asset bundle: %s\n", bundle);
        return 1;
    }
    TIM_IMAGE tim = {};
    QLP_HEAD* tex_buff = (QLP_HEAD*) malloc(lzpFileSize(lz_resources, lzp_index));
    lzpUnpackFile(tex_buff, lz_resources, lzp_index);
    const int file_count = qlpFileCount(tex_buff);
    // TODO: Can we just pre-load the indices of each texture for a given
    //       bundle? That way we can avoid needing to do strcmp lots of times
    //       for no reason.
    for (int i = 0; i < file_count; i++) {
        const QLP_FILE* file = qlpFileEntry(i, tex_buff);
        if (!strcmp(file->name, filename)
            && !GetTimInfo((uint32_t*) qlpFileAddr(i, tex_buff), &tim)) {
            DEBUG_LOG(
                "[TEXTURE] Loading: [Bundle: %s] [Name: %s] [Position: (%d,%d)] [Addr: %p] [Mode: 0x%x]\n",
                bundle,
                file->name,
                tim.prect->x,
                tim.prect->y,
                tim.caddr,
                tim.mode
            );
            assetLoadImage(&tim, texture);
            free(tex_buff);
            return 0;
        }
    }
    free(tex_buff);
    printf("[ERROR] No such file %s in asset bundle %s\n", filename, bundle);
    return 1;
}
