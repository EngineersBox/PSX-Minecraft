#pragma once

#ifndef PSX_MINECRAFT_BLOCK_ID_H
#define PSX_MINECRAFT_BLOCK_ID_H

#include "../../util/preprocessor.h"

#define BLOCK_LIST \
    ENUM_ENTRY_ORD(BLOCKID_AIR, 0), \
    ENUM_ENTRY_ORD(BLOCKID_STONE, 1), \
    ENUM_ENTRY_ORD(BLOCKID_GRASS, 2), \
    ENUM_ENTRY_ORD(BLOCKID_DIRT, 3)

typedef enum {
    ENUM_ENTRIES(BLOCK_LIST)
} EBlockID;

extern const char* EBLOCKID_NAMES[];

#define blockIdStringify(id) EBLOCKID_NAMES[(id)]

#endif // PSX_MINECRAFT_BLOCK_ID_H
