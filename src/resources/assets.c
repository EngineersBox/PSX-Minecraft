#include "assets.h"

#include <lzp/lzp.h>
#include <lzp/lzqlp.h>
#include <smd/smd.h>
#include <psxgpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../logging/logging.h"
#include "../hardware/cd.h"

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
    // TODO: Free this buffer after the static textures are loaded.
    //       Then move the dynamic textures like GUIs into a separate
    //       LZP resource that can be read at will for interactions
    //       (like opening an inventory) that require them.
    _lz_resources = (u8*) cdReadDataSync("\\ASSETS.LZP", CdlModeSpeed);
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
