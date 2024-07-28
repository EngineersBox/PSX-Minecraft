#include "primitive.h"

#include <psxgpu.h>
#include "../../util/inttypes.h"

INDEX indexRotate90(const INDEX* index, const bool direction) {
    if (direction) {
        return (INDEX) {
            .v0 = index->v2,
            .v1 = index->v0,
            .v2 = index->v3,
            .v3 = index->v1
        };
    }
    return (INDEX) {
        .v0 = index->v1,
        .v1 = index->v3,
        .v2 = index->v0,
        .v3 = index->v2
    };
}

void lineG2Render(const LINE_G2* line, const int ot_entry, RenderContext* ctx) {
    setLineG2(line);
    u32* ot_object = allocateOrderingTable(ctx, ot_entry);
    addPrim(ot_object, line);
}

void lineF2Render(const LINE_F2* line, const int ot_entry, RenderContext* ctx) {
    setLineF2(line);
    u32* ot_object = allocateOrderingTable(ctx, ot_entry);
    addPrim(ot_object, line);
}

void polyF4Render(const POLY_F4* pol4, int ot_entry, RenderContext* ctx) {
    setPolyF4(pol4);
    u32* ot_object = allocateOrderingTable(ctx, ot_entry);
    addPrim(ot_object, pol4);
}

void polyFT4Render(const POLY_FT4* pol4, int ot_entry, RenderContext* ctx) {
    setPolyFT4(pol4);
    u32* ot_object = allocateOrderingTable(ctx, ot_entry);
    addPrim(ot_object, pol4);
}

void tileRender(const TILE* tile, int ot_entry, RenderContext* ctx) {
    setTile(tile);
    u32* ot_object = allocateOrderingTable(ctx, ot_entry);
    addPrim(ot_object, tile);
}
