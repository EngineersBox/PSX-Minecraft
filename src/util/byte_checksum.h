#pragma once

#ifndef _PSXMC__UTIL__BYTE_CHECKSUM_H_
#define _PSXMC__UTIL__BYTE_CHECKSUM_H_

#include "inttypes.h"

u8 byteChecksum(const u8* data, u32 data_len);
u8 byteChunkedChecksum(const u8* data, u32 data_len);

#endif // _PSXMC__UTIL__BYTE_CHECKSUM_H_
