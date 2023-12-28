#pragma once

#ifndef PSX_MINECRAFT_POSITION_H
#define PSX_MINECRAFT_POSITION_H

#include <psxgte.h>

// TODO: IMPLEMENT THESE
static VECTOR worldToBlockPosition(const VECTOR* position);
static VECTOR worldToLocalBlockPosition(const VECTOR* position, const int size);
static VECTOR worldToChunkPosition(const VECTOR* position, const int size);

#endif // PSX_MINECRAFT_POSITION_H
