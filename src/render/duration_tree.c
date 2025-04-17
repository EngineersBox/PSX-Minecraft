#include "duration_tree.h"

#include <psxgpu.h>
#include <psxgte.h>
#include <stdio.h>

#include "../math/vector.h"
#include "../math/math_utils.h"
#include "../structure/cvector_utils.h"

#if isDebugTagEnabled(OVERLAY_DURATION_TREE)
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

DurationComponent* _durationTreeAddComponent(const char* name) {
    DurationComponent* tree = durationComponentCurrent();
    assert(tree != NULL);
    cvector_push_back(tree->components, (DurationComponent) {});
    DurationComponent* new_component = &tree->components[cvector_size(tree->components) - 1];
    new_component->name = name;
    return new_component;
}

void _durationTreeMakeCurrent(DurationComponent* tree) {
    duration_stack[0] = tree;
    duration_stack_next_index = 1;
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
    if (duration_stack_next_index > DURATION_STACK_MAX_DEPTH) {
        errorAbort(
            "Invalid duration stack index: %d\n",
            duration_stack_next_index
        );
        return;
    }
    duration_stack_next_index--;
    DurationComponent* node = duration_stack[duration_stack_next_index];
    duration_stack[duration_stack_next_index] = NULL;
    node->end = time_ms;
}

#define STACK_GRAPH_COLOUR_COUNT 11
static const CVECTOR stack_graph_colours[STACK_GRAPH_COLOUR_COUNT] = {
    /*vec3_rgb(0xff, 0, 0),*/
    /*vec3_rgb(0, 0xff, 0),*/
    /*vec3_rgb(0, 0, 0xff),*/
    /*vec3_rgb(0xff, 0xff, 0),*/
    /*vec3_rgb(0xff, 0, 0xff),*/
    /*vec3_rgb(0, 0xff, 0xff)*/
    vec3_rgb(216, 73, 147),
    vec3_rgb(136, 239, 47),
    vec3_rgb(29, 139, 183),
    vec3_rgb(216, 213, 2),
    vec3_rgb(206, 101, 41),
    vec3_rgb(130, 85, 198),
    vec3_rgb(22, 158, 139),
    vec3_rgb(84, 86, 201),
    vec3_rgb(173, 54, 32),
    vec3_rgb(242,228, 74),
    vec3_rgb(15, 64, 224)
};

#define DURATION_TREE_STACK_GRAPH_WIDTH 20
#define DURATION_TREE_STACK_GRAPH_HEIGHT 100
#define DURATION_TREE_STACK_GRAPH_X_POS (SCREEN_XRES - 5 - DURATION_TREE_STACK_GRAPH_WIDTH)
#define DURATION_TREE_STACK_GRAPH_Y_POS (9 * 6)

int selected_rendered_stack_index = 3;

void _durationTreeChangeStackIndex(const DurationComponent* tree, int adjustment) {
    const int component_count = cvector_size(tree->components);
    selected_rendered_stack_index = clamp(
        selected_rendered_stack_index + adjustment,
        0,
        component_count - 1
    );
}

void _durationTreeRender(const DurationComponent* tree,
                         RenderContext* ctx,
                         Transforms* transforms) {
    if (tree == NULL || cvector_size(tree->components) == 0) {
        // We were passed a non-existent tree
        // or there are no child components, and
        // thus no durations
        return;
    }
    // NOTE: Stack graph of dimensions 20x100.
    //       Label above graph shows total time.
    //       K,V above label shows selected index
    //       + duration time + percentage mapping
    //       to entry in graph.
    Timestamp total_duration = 0; //tree->end - tree->start;
    DurationComponent const* component = NULL;
    cvector_for_each_in(component, tree->components) {
        total_duration += component->end - component->start;
    }
    Timestamp selected_duration = 0;
    u8 selected_percentage = 0;
    const char* selected_name = NULL;
    u8 stack_offset = 0;
    int i = 0;
    u32* ot_object = allocateOrderingTable(ctx, 0);
    cvector_for_each_in(component, tree->components) {
        // Render each section of the stack graph
        const Timestamp duration = component->end - component->start;
        if (duration == 0) {
            continue;
        }
        const u8 percentage = (duration * 100) / total_duration;
        TILE* tile = (TILE*) allocatePrimitive(ctx, sizeof(TILE));
        setTile(tile);
        setXY0(
            tile,
            DURATION_TREE_STACK_GRAPH_X_POS,
            DURATION_TREE_STACK_GRAPH_Y_POS + stack_offset
        );
        setWH(
            tile,
            DURATION_TREE_STACK_GRAPH_WIDTH,
            percentage 
        );
        const CVECTOR* colour = &stack_graph_colours[i % STACK_GRAPH_COLOUR_COUNT];
        setRGB0(
            tile,
            colour->r >> (i != selected_rendered_stack_index),
            colour->g >> (i != selected_rendered_stack_index),
            colour->b >> (i != selected_rendered_stack_index)
        );
        addPrim(ot_object, tile);
        if (i == selected_rendered_stack_index) {
            selected_duration = duration;
            selected_percentage = percentage;
            selected_name = component->name;
            tile = (TILE*) allocatePrimitive(ctx, sizeof(TILE));
            setTile(tile);
            setXY0(
                tile,
                DURATION_TREE_STACK_GRAPH_X_POS + 2,
                DURATION_TREE_STACK_GRAPH_Y_POS - (9 * 5)
            );
            setWH(
                tile,
                8,
                8
            );
            setRGB0(
                tile,
                colour->r,
                colour->g,
                colour->b
            );
            addPrim(ot_object, tile);
        }
        stack_offset += percentage;
        i++;
    }
    // Background
    TILE* tile = (TILE*) allocatePrimitive(ctx, sizeof(TILE));
        setTile(tile);
        setXY0(
            tile,
            DURATION_TREE_STACK_GRAPH_X_POS,
            DURATION_TREE_STACK_GRAPH_Y_POS
        );
        setWH(
            tile,
            DURATION_TREE_STACK_GRAPH_WIDTH,
            DURATION_TREE_STACK_GRAPH_HEIGHT
        );
    setRGB0(tile, 0, 0, 0);
    addPrim(ot_object, tile);
    // Selected + total
    char str[256] = {0};
    sprintf(
        str,
        "Selected:\n- %s\n- %dms\n- %d%%\nTotal: %dms\n",
        selected_name,
        selected_duration,
        selected_percentage,
        total_duration
    );
    ctx->primitive = fontSort(
        ot_object,
        ctx->primitive,
        DURATION_TREE_STACK_GRAPH_X_POS - 70,
        DURATION_TREE_STACK_GRAPH_Y_POS - (9 * 5),
        true,
        str
    );
}

#endif
