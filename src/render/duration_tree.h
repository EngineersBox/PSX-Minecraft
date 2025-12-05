#pragma once

#include <stdint.h>
#ifndef _PSXMC__RENDER__DURATION_TREE_H_
#define _PSXMC__RENDER__DURATION_TREE_H_

// TODO: Write a blog post about this duration tree implementation,
//       why it exists, how it works and examples.

#include "../structure/cvector.h"
#include "../hardware/counters.h"
#include "../debug/debug_defines.h"
#include "font.h"
#include "render_context.h"
#include "transforms.h"

#define DURATION_COMPONENT_NAME_MAX_LEN 8
#define DURATION_STACK_MAX_DEPTH 10

typedef struct DurationComponent {
    Timestamp start;
    Timestamp end;
    const char* name;
    cvector(struct DurationComponent) components;
} DurationComponent;

typedef struct DurationComponentIndex {
    bool init: 1;
    size_t index: SIZE_WIDTH - 1;
} DurationComponentIndex;

#if isDebugFlagEnabled(OVERLAY_DURATION_TREE)
extern DurationComponent render_duration_tree;
extern DurationComponent update_duration_tree;
extern u8 duration_stack_next_index;
extern DurationComponent* duration_stack[DURATION_STACK_MAX_DEPTH];
extern int selected_rendered_stack_index;

void durationTreesInit0();
void durationTreesDestroy0();

DurationComponentIndex durationTreeAddComponent0(const char* name);
void durationTreeMakeCurrent0(DurationComponent* tree);
void durationComponentStart0(DurationComponentIndex* index);
void durationComponentEnd0();
void durationTreeChangeStackIndex0(const DurationComponent* tree, int adjustment);
void durationTreeRender0(const DurationComponent* tree, RenderContext* ctx, Transforms* transforms);

#define durationTreesInit durationTreesInit0
#define durationTreesDestroy durationTreesDestroy0
#define durationTreeAddComponent durationTreeAddComponent0
#define durationTreeMakeCurrent durationTreeMakeCurrent0
#define durationComponentCurrent() (duration_stack_next_index == 0 \
    ? NULL \
    : duration_stack[duration_stack_next_index - 1])
#define durationComponentCurrentAtIndex(idx) &durationComponentCurrent()->components[(idx)]
#define durationComponentStart durationComponentStart0
#define durationComponentEnd durationComponentEnd0
#define durationTreeChangeStackIndex durationTreeChangeStackIndex0
#define durationTreeRender durationTreeRender0

#define DEFN_DURATION_COMPONENT(defn_name) static DurationComponentIndex defn_name##_duration = (DurationComponentIndex){0} 
#define durationComponentInitOnce(defn_name, name) ({ \
    if (!defn_name##_duration.init) { \
        defn_name##_duration = durationTreeAddComponent(name); \
    } \
})
#else
#define durationTreesInit() ({})
#define durationTreesDestroy() ({})
#define durationTreeAddComponent(name) ({})
#define durationTreeMakeCurrent() ({})
#define durationComponentCurrent() ({})
#define durationComponentCurrentAtIndex(idx) ({})
#define durationComponentStart(node) ({})
#define durationComponentEnd(node) ({})

#define DEFN_DURATION_COMPONENT(name)
#define durationComponentInitOnce(defn_name, name) ({})
#define durationTreeChangeStackIndex(tree, adjustment) ({})
#define durationTreeRender(tree, ctx, transforms) ({})
#endif

#endif // _PSXMC__RENDER__DURATION_TREE_H_
