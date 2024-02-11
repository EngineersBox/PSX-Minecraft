#include "primitive.h"

void lineG2Render(const LINE_G2* line, const int ot_entry, RenderContext* ctx) {
    setLineG2(line);
    uint32_t* ot_object = allocateOrderingTable(ctx, ot_entry);
    addPrim(ot_object, line);
}

void polyF4Render(const POLY_F4* pol4, int ot_entry, RenderContext* ctx) {
    setPolyF4(pol4);
    uint32_t* ot_object = allocateOrderingTable(ctx, ot_entry);
    addPrim(ot_object, pol4);
}