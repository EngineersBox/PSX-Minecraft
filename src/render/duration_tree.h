#pragma once

#ifndef _PSXMC__RENDER__DURATION_TREE_H_
#define _PSXMC__RENDER__DURATION_TREE_H_

#include "../structure/cvector.h"
#include "../hardware/counters.h"
#include "debug_defines.h"

#define DURATION_COMPONENT_NAME_MAX_LEN 8
#define DURATION_STACK_MAX_DEPTH 10

typedef struct DurationComponent {
    Timestamp start;
    Timestamp end;
    const char* name;
    cvector(struct DurationComponent) components;
} DurationComponent;

#if isOverlayEnabled(DURATION_TREE)
extern DurationComponent render_duration_tree;
extern DurationComponent update_duration_tree;
extern u8 duration_stack_next_index;
extern DurationComponent* duration_stack[DURATION_STACK_MAX_DEPTH];

extern int selected_rendered_stack_index;

void _durationTreesInit();
void _durationTreesDestroy();

DurationComponent* _durationTreeAddComponent(const char* name);
void _durationTreeMakeCurrent(DurationComponent* tree);
void _durationComponentStart(DurationComponent* node);
void _durationComponentEnd();

#define durationTreesInit _durationTreesInit
#define durationTreesDestroy _durationTreesDestroy
#define durationTreeAddComponent _durationTreeAddComponent
#define durationTreeMakeCurrent _durationTreeMakeCurrent
#define durationComponentCurrent() duration_stack_next_index == 0 \
    ? NULL \
    : duration_stack[duration_stack_next_index - 1]
#define durationComponentStart _durationComponentStart
#define durationComponentEnd _durationComponentEnd

#define DEFN_DURATION_COMPONENT(defn_name) static DurationComponent* defn_name##_duration = NULL
#define durationComponentInitOnce(defn_name, name) ({ \
    if (defn_name##_duration == NULL) { \
        defn_name##_duration = durationTreeAddComponent(name); \
    } \
})
#else
#define durationTreesInit() ({})
#define durationTreesDestroy() ({})
#define durationTreeAddComponent(name) ({})
#define durationTreeMakeCurrent() ({})
#define durationComponentCurrent() ({})
#define durationComponentStart(node) ({})
#define durationComponentEnd(node) ({})

#define DEFN_DURATION_COMPONENT(name) ({})
#define durationComponentInitOnce(defn_name, name) ({})
#endif

#endif // _PSXMC__RENDER__DURATION_TREE_H_
