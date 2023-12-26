#include "assets.h"

#include <lzp/lzp.h>
#include <lzp/lzqlp.h>
#include <smd/smd.h>
#include <psxgpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

void assetsLoad() {
    int *ttim;
    TIM_IMAGE tim;
    int i = lzpSearchFile("textures", lz_resources);
    QLP_HEAD* tex_buff = (QLP_HEAD*) malloc(lzpFileSize(lz_resources, i));
    lzpUnpackFile(tex_buff, lz_resources, i);
    const int file_count = qlpFileCount(tex_buff);
    printf("Loading %d assets\n", file_count);
    for (int j = 0; j < file_count; j++) {
        const QLP_FILE* file = qlpFileEntry(i, tex_buff);
        printf("Current file: %s\n", file->name);
        if (!GetTimInfo((u_long*) qlpFileAddr(j, tex_buff), &tim)) {
            printf(
                "Loading image: [Name: %s] [Addr: %p] [Mode: 0x%x]\n",
                file->name,
                tim.caddr,
                tim.mode
            );
            assetLoadImage(&tim);
        }
    }
    free(tex_buff);
}

void assetLoadImage(const TIM_IMAGE* tim) {
    LoadImage(tim->prect, tim->paddr);
    if (tim->mode & 0x8) {
        LoadImage(tim->crect, tim->caddr);
    }
}