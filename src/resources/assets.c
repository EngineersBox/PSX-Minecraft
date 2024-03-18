#include "assets.h"

#include <lzp/lzp.h>
#include <lzp/lzqlp.h>
#include <smd/smd.h>
#include <psxgpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

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

void _loadTextures(const int lzp_index) {
    // TODO: Something weird going on here, print statement shows GUI twice, but VRAM
    //       view shows both terrain and GUI
    TIM_IMAGE tim;
    QLP_HEAD* tex_buff = (QLP_HEAD*) malloc(lzpFileSize(lz_resources, lzp_index));
    lzpUnpackFile(tex_buff, lz_resources, lzp_index);
    const int file_count = qlpFileCount(tex_buff);
    printf("[TEXTURE] Loading %d texture(s)\n", file_count);
    if (file_count > 0) {
        textures = (Texture*) calloc(file_count, sizeof(Texture));
    }
    for (int i = 0; i < file_count; i++) {
        const QLP_FILE* file = qlpFileEntry(lzp_index, tex_buff);
        if (!GetTimInfo((uint32_t*) qlpFileAddr(i, tex_buff), &tim)) {
            printf(
                "[TEXTURE: %d] Loading: [Name: %s] [Addr: %p] [Mode: 0x%x]\n",
                i,
                file->name,
                tim.caddr,
                tim.mode
            );
            assetLoadImage(&tim, i);
        }
    }
    free(tex_buff);
}

void assetLoadImage(const TIM_IMAGE* tim, const int index) {
    LoadImage(tim->prect, tim->paddr);
    if (tim->mode & 0x8) {
        LoadImage(tim->crect, tim->caddr);
    }
    textures[index].tpage = getTPage(
        tim->mode,
        1,
        tim->prect->x,
        tim->prect->y
    );
    textures[index].clut = getClut(
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
    for (const AssetBundle* bundle = &ASSET_BUNDLES[0]; bundle->name != NULL; bundle++) {
        const int lzp_index = lzpSearchFile(bundle->name, lz_resources);
        if (lzp_index < 0) {
            printf("Asset bundle '%s' not found, skipping\n", bundle->name);
            continue;
        }
        printf("Found asset bundle '%s', loading\n", bundle->name);
        bundle->loader(lzp_index);
    }
}

void assetsFree() {
    for (const AssetBundle* bundle = &ASSET_BUNDLES[0]; bundle->name != NULL; bundle++) {
        bundle->freer();
    }
}
