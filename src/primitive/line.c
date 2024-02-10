#include "line.h"

void lineG2Render(const LINE_G2* line, int ot_entry, RenderContext* ctx) {
    setLineG2(line);
    uint32_t* ot_object = allocateOrderingTable(ctx, ot_entry);
    addPrim(ot_object, line);
}