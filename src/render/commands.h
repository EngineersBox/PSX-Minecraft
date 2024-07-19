#pragma once

#ifndef _PSXMC__RENDER__COMMANDS_H_
#define _PSXMC__RENDER__COMMANDS_H_

#include <psxgpu.h>

#define setTextured(p, tex) ((tex) ? (getcode(p) |= 0b100) : (getcode(p) & ~0b100))
#define setTransparency(p, transp) ((transp) ? (getcode(p) |= 0b10) : (getcode(p) &= ~0b10))
#define setModulated(p, mod) ((mod) ? (getcode(p) &= ~0b1) : (getcode(p) |= 0b1))

#endif // _PSXMC__RENDER__COMMANDS_H_
