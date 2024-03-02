#pragma once

#ifndef PSX_MINECRAFT_BLOCK_ID_H
#define PSX_MINECRAFT_BLOCK_ID_H

#include "../../util/preprocessor.h"

#define MK_BLOCK_LIST(f) \
    f(BLOCKID_AIR) \
    f(BLOCKID_STONE) \
    f(BLOCKID_DIRT) \
    f(BLOCKID_GRASS)

typedef enum {
    BLOCKID_NONE = -1,
    MK_BLOCK_LIST(P99_ENUM_ENTRY)
} EBlockID;

extern const char* EBLOCKID_NAMES[];

#define blockIdStringify(id) (id) == BLOCKID_NONE ? "BLOCKID_NONE" : EBLOCKID_NAMES[(id)]

#endif // PSX_MINECRAFT_BLOCK_ID_H
