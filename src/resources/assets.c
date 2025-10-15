#include "assets.h"

#include <lzp/lzp.h>
#include <lzp/lzqlp.h>
#include <smd/smd.h>
#include <psxgpu.h>
#include <stdio.h>
#include "../core/std/stdlib.h"
#include <string.h>
#include <sys/types.h>

#include "../logging/logging.h"
#include "../hardware/cd.h"
#include "../util/preprocessor.h"
#include "../util/memory.h"
#include "asset_indices.h"

typedef void* (*AssetLoad)(const void* ctx);
typedef void (*AssetFree)(void* ctx);

typedef struct {
    AssetLoad load;
    AssetFree free;
    char* name;
    char* pack;
} AssetBundle;

FWD_DECL static void* _loadTextures(const void* ctx);
FWD_DECL static void _freeTextures(void* ctx);

FWD_DECL static void* _loadDynamicTextures(const void* ctx);

AssetBundle ASSET_BUNDLES[ASSET_BUNDLES_COUNT] = {
    [ASSET_BUNDLE__STATIC]=(AssetBundle) {
        .load = _loadTextures,
        .free = _freeTextures,
        .name = "static",
        .pack = "\\STATIC.LZP"
    },
    [ASSET_BUNDLE__GUI]=(AssetBundle) {
        .load = _loadDynamicTextures,
        .free = free,
        .name = "gui",
        .pack = "\\GUI.LZP"
    },
    [ASSET_BUNDLE__START]=(AssetBundle) {
        .load = _loadDynamicTextures,
        .free = free,
        .name = "start",
        .pack = "\\START.LZP"
    },
    [ASSET_BUNDLE__MENU]=(AssetBundle) {
        .load = _loadDynamicTextures,
        .free = free,
        .name = "menu",
        .pack = "\\MENU.LZP"
    },
    [ASSET_BUNDLES_COUNT - 1]=(AssetBundle) { NULL, NULL, NULL, NULL }
};

Texture* textures;
bool assets_loaded = false;
u8* _lz_resources = NULL;

static void* _loadTextures(const void* ctx) {
    const int lzp_index = (int) ctx;
    TIM_IMAGE tim = {0};
    QLP_HEAD* tex_buff = malloc(lzpFileSize(lz_resources, lzp_index));
    zeroed(tex_buff);
    lzpUnpackFile(tex_buff, lz_resources, lzp_index);
    const int file_count = qlpFileCount(tex_buff);
    DEBUG_LOG("[TEXTURE] Loading %d texture(s)\n", file_count);
    if (file_count > 0) {
        textures = calloc(file_count, sizeof(Texture));
        zeroed(textures);
    }
    for (int i = 0; i < file_count; i++) {
#if isDebugEnabled()
        const QLP_FILE* file = qlpFileEntry(i, tex_buff);
#endif
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

static void _freeTextures(UNUSED void* ctx) {
    free(textures);
}

void assetsLoad() {
    const AssetBundle* bundle = &ASSET_BUNDLES[ASSET_BUNDLE__STATIC];
    _lz_resources = (u8*) cdReadDataSync(bundle->pack, CdlModeSpeed);
    const int lzp_index = lzpSearchFile(bundle->name, lz_resources);
    if (lzp_index < 0) {
        errorAbort("[ERROR] Asset bundle not found: %s\n", bundle->name);
        return;
    }
    DEBUG_LOG("Found asset bundle '%s' @ %d, loading\n", bundle->name, lzp_index);
    bundle->load((void*) lzp_index);
    assets_loaded = true;
    free(_lz_resources);
    _lz_resources = NULL;
}

void assetsFree() {
    ASSET_BUNDLES[ASSET_BUNDLE__STATIC].free(NULL);
    assets_loaded = false;
}

static void* _loadDynamicTextures(const void* ctx) {
    const AssetBundle* bundle = ctx;
    return cdReadDataSync(bundle->pack, CdlModeSpeed);
}

void assetLoadTextureDirect(const size_t bundle, const int file_index, Texture* texture) {
    if (bundle == 0) {
        texture->tpage = textures[file_index].tpage;
        texture->clut = textures[file_index].clut;
        return;
    } else if (bundle > ASSET_BUNDLES_COUNT) {
        errorAbort("[ERROR] No such asset bundle at index: %d\n", bundle);
        return;
    }
    const AssetBundle* asset_bundle = &ASSET_BUNDLES[bundle];
    LZP_HEAD* archive = asset_bundle->load(asset_bundle);
    const int lzp_index = lzpSearchFile(asset_bundle->name, archive);
    if (lzp_index < 0) {
        errorAbort("[ERROR] Asset bundle not found: %s\n", asset_bundle->name);
        return;
    }
    QLP_HEAD* tex_buff = malloc(lzpFileSize(archive, lzp_index));
    zeroed(tex_buff);
    lzpUnpackFile(tex_buff, archive, lzp_index);
    TIM_IMAGE tim = {0};
    if (file_index >= qlpFileCount(tex_buff)) {
        errorAbort(
            "[ERROR] No such file index %d in asset bundle %d\n",
            file_index,
            asset_bundle->name
        );
        return;
    }
    const QLP_FILE* file = qlpFileEntry(file_index, tex_buff);
    if (file == NULL) {
        errorAbort(
            "[ERROR] No such file index %d in asset bundle %s\n",
            file_index,
            asset_bundle->name
        );
        return;
    }
    if (GetTimInfo((uint32_t*) qlpFileAddr(file_index, tex_buff), &tim)) {
        errorAbort("[ERROR] Failed to retrieve TIM info for file %s\n", file->name);
        return;
    }
    assetLoadImage(&tim, texture);
    asset_bundle->free(archive);
    free(tex_buff);
    DEBUG_LOG("Loaded asset bundle: %s\n", asset_bundle->name);
}
