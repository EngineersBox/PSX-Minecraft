#pragma once

#ifndef _PSXMC__HARDWARE__CD_H_
#define _PSXMC__HARDWARE__CD_H_

#include <psxcd.h>
#include <stdio.h>
#include <stddef.h>

#include "../util/inttypes.h"
#include "../logging/logging.h"

void* cdReadDataSync(const char* filename, CdlModeFlag mode);
void* cdReadFileSync(const CdlFILE* file, CdlModeFlag mode);

#endif // _PSXMC__HARDWARE__CD_H_
