#pragma once

#ifndef _PSXMC__RENDER__DURATION_TREE_H_
#define _PSXMC__RENDER__DURATION_TREE_H_

#include "../structure/cvector.h"
#include "../hardware/counters.h"
#include "debug_defines.h"

#define DURATION_COMPONENT_NAME_MAX_LEN 8

typedef struct DurationComponent {
    Timestamp start;
    Timestamp end;
    char name[DURATION_COMPONENT_NAME_MAX_LEN];
    cvector(struct DurationComponent) components;
} DurationComponent;

#if isOverlayEnabled(DURATION_TREE)
extern DurationComponent render_duration_tree;
extern DurationComponent update_duration_tree;
extern DurationComponent* current_node;

void _durationTreesInit();
void _durationTreesDestroy();

DurationComponent* _durationTreeAddComponent(DurationComponent* node);

#define durationTreesInit _durationTreesInit
#define durationTreesDestroy _durationTreesDestroy
#define durationTreeAddComponent _durationTreeAddComponent
#define durationComponentStart(node) (node)->start = time_ms
#define durationComponentEnd(node) (node)->end = time_ms
#else
#define durationTreesInit() ({})
#define durationTreesDestroy() ({})
#define durationTreeAddComponent(node) ({})
#define durationComponentStart(node) ({})
#define durationComponentEnd(node) ({})
#endif

#endif // _PSXMC__RENDER__DURATION_TREE_H_
