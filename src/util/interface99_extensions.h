#pragma once

#ifndef PSX_MINECRAFT_INTERFACE99_EXTENSIONS_H
#define PSX_MINECRAFT_INTERFACE99_EXTENSIONS_H

#include <interface99.h>

#include "preprocessor.h"

#define interface_extended(iface, ...) interface_extended99(iface, __VA_ARGS__)
#define interface_extended99(iface, ...) ML99_EVAL(IFACE99_interface_extended_IMPL(iface, __VA_ARGS__))
#define IFACE99_interface_extended_IMPL(iface, ...)                                                \
    ML99_TERMS(                                                                                    \
        v(typedef struct iface##VTable iface##VTable;),                                            \
        v(typedef struct iface iface;),                                                            \
        ML99_semicoloned(ML99_struct(v(iface##VTable), IFACE99_PRIV_genVTableFields(iface))),      \
        v(struct iface {                                                                           \
            void *self;                                                                            \
            const iface##VTable *vptr;                                                             \
            __VA_ARGS__;                                                                           \
        }))

#endif // PSX_MINECRAFT_INTERFACE99_EXTENSIONS_H
