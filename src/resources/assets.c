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
#include "asset_indices.h"

typedef void* (*AssetLoad)(const void* ctx);

typedef void (*AssetFree)(const void* ctx);

typedef struct {
    AssetLoad load;
    AssetFree free;
    char* name;
} AssetBundle;

static void* _loadTextures(const void* ctx);
static void _freeTextures(const void* ctx);

static void* _loadDynamicTextures(const void* ctx);
static void _freeDynamicTextures(const void* ctx);

AssetBundle ASSET_BUNDLES[ASSET_BUNDLES_COUNT] = {
    [ASSET_BUNDLE__STATIC]=(AssetBundle) {
        .load = _loadTextures,
        .free = _freeTextures,
        .name = "static"
    },
    [ASSET_BUNDLE__GUI]=(AssetBundle) {
        .load = _loadDynamicTextures,
        .free = _freeDynamicTextures,
        .name = "\\GUI.LZP"
    },
    [ASSET_BUNDLES_COUNT - 1]=(AssetBundle) { NULL, NULL, NULL }
};

Texture* textures;
bool assets_loaded = false;
u8* _lz_resources = NULL;

static void* _loadTextures(const void* ctx) {
    const int lzp_index = (int) ctx;
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
    return NULL;
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

static void _freeTextures(const void* ctx) {
    free(textures);
}

void assetsLoad() {
    _lz_resources = (u8*) cdReadDataSync("\\STATIC.LZP", CdlModeSpeed);
    const AssetBundle* bundle = &ASSET_BUNDLES[0];
    const int lzp_index = lzpSearchFile(bundle->name, lz_resources);
    if (lzp_index < 0) {
        errorAbort("[ERROR] Asset bundle not found: %s\n", bundle->name);
        return;
    }
    DEBUG_LOG("Found asset bundle '%s', loading\n", bundle->name);
    bundle->load((void*) lzp_index);
    assets_loaded = true;
    free(_lz_resources);
    _lz_resources = NULL;
}

void assetsFree() {
    ASSET_BUNDLES[0].free(NULL);
    assets_loaded = false;
}

static void* _loadDynamicTextures(const void* ctx) {
    const AssetBundle* bundle = ctx;
    return cdReadDataSync(bundle->name, CdlModeSpeed);
}

static void _freeDynamicTextures(const void* ctx) {
    free((u8*) ctx);
}

int assetLoadTextureDirect(const size_t bundle, const int file_index, Texture* texture) {
    if (bundle == 0) {
        texture->tpage = textures[file_index].tpage;
        texture->clut = textures[file_index].clut;
        return 0;
    } else if (bundle > ASSET_BUNDLES_COUNT) {
        errorAbort("[ERROR] No such asset bundle at index: %d\n", bundle);
        return 1;
    }
    DEBUG_LOG("PARAMS Bundle: %d File index: %d Texture: %p\n", bundle, file_index, texture);
    const AssetBundle* asset_bundle = &ASSET_BUNDLES[bundle];
    const LZP_HEAD* archive = asset_bundle->load(asset_bundle);
    DEBUG_LOG("Loaded archive: %p\n", archive);
    TIM_IMAGE tim = {};
    const int fsize = lzpFileSize(archive, 0);
    DEBUG_LOG("File size: %d\n", fsize);
    QLP_HEAD* tex_buff = (QLP_HEAD*) malloc(fsize);
    DEBUG_LOG("Allocated buffer: %p\n", tex_buff);
    lzpUnpackFile(tex_buff, archive, file_index);
    DEBUG_LOG("Unpacked file\n");
    if (file_index >= qlpFileCount(tex_buff)) {
        errorAbort(
            "[ERROR] No such file index %d in asset bundle %d\n",
            file_index,
            asset_bundle->name
        );
        return 1;
    }
    const QLP_FILE* file = qlpFileEntry(file_index, tex_buff);
    if (file == NULL) {
        errorAbort(
            "[ERROR] No such file index %d in asset bundle %d\n",
            file_index,
            asset_bundle->name
        );
        return 1;
    }
    DEBUG_LOG("Retrieved file entry\n");
    if (GetTimInfo((uint32_t*) qlpFileAddr(file_index, tex_buff), &tim)) {
        errorAbort("[ERROR] Failed to retrieve TIM info for file %s\n", file->name);
        return 1;
    }
    DEBUG_LOG("Retrieved TIM info\n");
    assetLoadImage(&tim, texture);
    DEBUG_LOG("Loaded image\n");
    asset_bundle->free(archive);
    DEBUG_LOG("Freed archive\n");
    free(tex_buff);
    return 0;
}
