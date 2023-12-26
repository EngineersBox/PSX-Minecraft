#include "assets.h"

#include <lzp/lzp.h>
#include <lzp/lzqlp.h>
#include <smd/smd.h>
#include <psxgpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

typedef void (*AssetLoader)(int lzp_index);

typedef struct {
    AssetLoader loader;
    char *name;
} AssetBundle;

inline void _loadTextures(int lzp_index);
inline void _loadModels(int lzp_index);

const int ASSET_BUNDLES_SIZE = 2;
AssetBundle ASSET_BUNDLES[] = {
    (AssetBundle) {
        .loader = _loadTextures,
        .name = "textures"
    },
    (AssetBundle) {
        .loader = _loadModels,
        .name = "models"
    }
};

void _loadTextures(const int lzp_index) {
    TIM_IMAGE tim;
    QLP_HEAD *tex_buff = (QLP_HEAD *) malloc(lzpFileSize(lz_resources, lzp_index));
    lzpUnpackFile(tex_buff, lz_resources, lzp_index);
    const int file_count = qlpFileCount(tex_buff);
    printf("[TEXTURE] Loading %d texture(s)\n", file_count);
    for (int j = 0; j < file_count; j++) {
        const QLP_FILE *file = qlpFileEntry(lzp_index, tex_buff);
        if (!GetTimInfo((u_long *) qlpFileAddr(j, tex_buff), &tim)) {
            printf(
                "[TEXTURE] Loading: [Name: %s] [Addr: %p] [Mode: 0x%x]\n",
                file->name,
                tim.caddr,
                tim.mode
            );
            assetLoadImage(&tim);
        }
    }
    free(tex_buff);
}

void _loadModels(int lzp_index) {
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

void assetLoadImage(const TIM_IMAGE *tim) {
    LoadImage(tim->prect, tim->paddr);
    if (tim->mode & 0x8) {
        LoadImage(tim->crect, tim->caddr);
    }
}

void assetLoadModel(const SMD *smd) {
}
