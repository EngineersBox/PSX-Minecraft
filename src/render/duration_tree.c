#include "duration_tree.h"

#if isOverlayEnabled(DURATION_TREE)
#include <stdlib.h>

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
DurationComponent* current_node = NULL;

static void __durationComponentFree(void* ctx) {
    DurationComponent* component = ctx;
    cvector_free(component->components);
}

void _durationTreesInit() {
    render_duration_tree.components = NULL;
    cvector_init(render_duration_tree.components, 0, __durationComponentFree);
    update_duration_tree.components = NULL;
    cvector_init(update_duration_tree.components, 1, __durationComponentFree);
}

void _durationTreesDestroy() {
    cvector_free(render_duration_tree.components);
    cvector_free(update_duration_tree.components);
}

DurationComponent* _durationTreeAddComponent(DurationComponent* node) {
    cvector_push_back(node->components, (DurationComponent) {});
    return &node->components[cvector_size(node->components) - 1];
}
#endif
