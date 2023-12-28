#include "chunk_mesh.h"

#include <stdlib.h>

void __primtiveDestructor(void* elem) {
    memset(elem, 0, sizeof(SMD_PRIM));
}

void __svectorDestructor(void* elem) {
    memset(elem, 0, sizeof(SVECTOR));
}

void chunkMeshInit(ChunkMesh* mesh) {
    cvector_init(mesh->primitives, 0, __primtiveDestructor);
    cvector_init(mesh->vertices, 0, __svectorDestructor);
    cvector_init(mesh->normals, 0, __svectorDestructor);
}

void chunkMeshDestroy(const ChunkMesh* mesh) {
    cvector_free(mesh->primitives);
    cvector_free(mesh->vertices);
    cvector_free(mesh->normals);
}

void chunkMeshClear(ChunkMesh* mesh) {
    cvector_clear(mesh->primitives);
    cvector_clear(mesh->vertices);
    cvector_clear(mesh->normals);
    mesh->smd = {0};
}