#include "assets.h"

#include <lzp/lzp.h>
#include <lzp/lzqlp.h>
#include <smd/smd.h>
#include <psxgpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef void (*AssetLoader)(int lzp_index);
typedef void (*AssetFreer)(void);

typedef struct {
    AssetLoader loader;
    AssetFreer freer;
    char *name;
} AssetBundle;

inline void _loadTextures(int lzp_index);
inline void _freeTextures();

inline void _loadModels(int lzp_index);
inline void _freeModels();

#define ASSET_BUNDLES_SIZE 2
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
    }
};

Texture* textures;

void _loadTextures(const int lzp_index) {
    TIM_IMAGE tim;
    QLP_HEAD *tex_buff = (QLP_HEAD*) malloc(lzpFileSize(lz_resources, lzp_index));
    lzpUnpackFile(tex_buff, lz_resources, lzp_index);
    const int file_count = qlpFileCount(tex_buff);
    printf("[TEXTURE] Loading %d texture(s)\n", file_count);
    if (file_count > 0) {
        textures = (Texture*) malloc(file_count * sizeof(Texture));
    }
    for (int j = 0; j < file_count; j++) {
        const QLP_FILE *file = qlpFileEntry(lzp_index, tex_buff);
        if (!GetTimInfo((u_long *) qlpFileAddr(j, tex_buff), &tim)) {
            printf(
                "[TEXTURE: %d] Loading: [Name: %s] [Addr: %p] [Mode: 0x%x]\n",
                j,
                file->name,
                tim.caddr,
                tim.mode
            );
            assetLoadImage(&tim, j);
        }
    }
    free(tex_buff);
}

void assetLoadImage(const TIM_IMAGE *tim, const int index) {
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

void assetLoadModel(const SMD *smd) {
}

void _freeModels() {

}

void assetsLoad() {
    for (int i = 0; i < ASSET_BUNDLES_SIZE; i++) {
        const AssetBundle bundle = ASSET_BUNDLES[i];
        const int lzp_index = lzpSearchFile(bundle.name, lz_resources);
        if (lzp_index < 0) {
            printf("Asset bundle '%s' not found, skipping\n", bundle.name);
            continue;
        }
        printf("Found asset bundle '%s', loading\n", bundle.name);
        bundle.loader(lzp_index);
    }
}

void assetsFree() {
    for (int i = 0; i < ASSET_BUNDLES_SIZE; i++) {
        const AssetBundle bundle = ASSET_BUNDLES[i];
        bundle.freer();
    }
}