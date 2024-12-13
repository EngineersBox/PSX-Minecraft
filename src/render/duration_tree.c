#include "duration_tree.h"

#if isOverlayEnabled(DURATION_TREE)
#include <stdlib.h>

#include "../logging/logging.h"

DurationComponent render_duration_tree = (DurationComponent) {
    .start = 0,
    .end = 0,
    .name = "Total",
    .components = NULL
};
DurationComponent update_duration_tree = (DurationComponent) {
    .start = 0,
    .end = 0,
    .name = "Total",
    .components = NULL
};
u8 duration_stack_next_index = 0;
DurationComponent* duration_stack[DURATION_STACK_MAX_DEPTH] = {0};

static void __durationComponentFree(void* ctx) {
    DurationComponent* component = ctx;
    cvector_free(component->components);
}

void _durationTreesInit() {
    render_duration_tree.components = NULL;
    cvector_init(render_duration_tree.components, 0, __durationComponentFree);
    update_duration_tree.components = NULL;
    cvector_init(update_duration_tree.components, 0, __durationComponentFree);
    for (u8 i = 0; i < DURATION_STACK_MAX_DEPTH; i++) {
        duration_stack[i] = NULL;
    }
    duration_stack_next_index = 0;
}

void _durationTreesDestroy() {
    cvector_free(render_duration_tree.components);
    cvector_free(update_duration_tree.components);
}

DurationComponent* _durationTreeAddComponent(DurationComponent* node) {
    cvector_push_back(node->components, (DurationComponent) {});
    return &node->components[cvector_size(node->components) - 1];
}

void _durationComponentStart(DurationComponent* node) {
    if (duration_stack_next_index >= DURATION_STACK_MAX_DEPTH) {
        errorAbort(
            "Exceeded max duration stack depth of %d\n",
            DURATION_STACK_MAX_DEPTH
        );
        return;
    }
    duration_stack[duration_stack_next_index++] = node;
    node->start = time_ms;
}

void _durationComponentEnd() {
    if (duration_stack_next_index < 0) {
        errorAbort("Negative duration stack index\n");
        return;
    }
    duration_stack_next_index--;
    DurationComponent* node = duration_stack[duration_stack_next_index];
    duration_stack[duration_stack_next_index] = NULL;
    node->end = time_ms;
}

#endif
