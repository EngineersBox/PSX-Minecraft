#pragma once

#ifndef PSXMC_INTERFACE99_EXTENSIONS_H
#define PSXMC_INTERFACE99_EXTENSIONS_H

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

#define VSelfRef(type) ((type) iface99_self)

#define VCAST(type, obj) VCAST99(type, obj)
#define VCAST99(type, obj) ((type)(obj).self)
#define VCAST_PTR(type, ptr) VCAST_PTR99(type, ptr)
#define VCAST_PTR99(type, ptr) ((type)(ptr)->self)

#define DYN_PTR(ptr, implementer, iface, ...) DYN_PTR99(ptr, implementer, iface, __VA_ARGS__)
#define DYN_LIT_PTR(ptr, implementer, iface, ...) DYN_LIT_PTR99(ptr, implementer, iface, __VA_ARGS__)
#define DYN_PTR99(ptr, implementer, iface, ...) ({ \
    (ptr)->self = (void *)(__VA_ARGS__); \
    (ptr)->vptr = &VTABLE99(implementer, iface); \
})
#define DYN_LIT_PTR99(ptr, implementer, iface, ...) DYN_PTR99(ptr, implementer, iface, &(implementer)__VA_ARGS__)
#define IMPLEMENTER_VTABLE(ptr) ((ptr)->vptr)
#define DYN_VINIT(ptr, src_ptr, ...) ({ \
    (ptr)->self = (void*)(__VA_ARGS__); \
    (ptr)->vptr = (src_ptr)->vptr; \
})

#define INSTANCEOF(obj, implementer, iface) INSTANCEOF99(obj, implementer, iface)
#define INSTANCEOF99(obj, implementer, iface) ((type).vptr == &VTABLE99(implementer, iface))
#define INSTANCEOF_PTR(ptr, implementer, iface) INSTANCEOF_PTR99(ptr, implementer, iface)
#define INSTANCEOF_PTR99(ptr, implementer, iface) ((type)->vptr == &VTABLE99(implementer, iface))

#endif // PSXMC_INTERFACE99_EXTENSIONS_H
