#include "meshing.h"

#include "../../../blocks/blocks.h"

const INDEX QUAD_INDICES[FACE_DIRECTION_COUNT] = {
    [0]={3,2,1,0},
    [1]={1,0,3,2},
    [2]={3,2,1,0},
    [3]={2,3,0,1},
    [4]={2,3,0,1},
    [5]={3,2,1,0}
};

MeshPrimitive* createPrimitive(ChunkMesh* mesh,
                               const PlaneMeshingDataKey* data,
                               const u32 width,
                               const u32 height,
                               const Texture* texture_override,
                               const TextureAttributes texture_attributes_override[FACE_DIRECTION_COUNT]) {
    Mesh* face_dir_mesh = &mesh->face_meshes[data->face];
    cvector(MeshPrimitive) prims = face_dir_mesh->p_prims;
    cvector_push_back(prims, (MeshPrimitive){0});
    // cvector can be reallocated by pushing a
    // new element, thus we should re-set it to
    // the mesh object that owns it.
    face_dir_mesh->p_prims = prims;
    MeshPrimitive* primitive = &prims[face_dir_mesh->n_prims++];
    primitive->type = MESH_PRIM_TYPE_QUAD;
    const Texture* texture = texture_override != NULL
        ? texture_override
        : &textures[ASSET_TEXTURE__STATIC__TERRAIN];
    primitive->tpage = texture->tpage;
    primitive->clut = texture->clut;
    const TextureAttributes* attributes = texture_attributes_override != NULL
        ? &texture_attributes_override[data->face]
        : &blockGetFaceAttributes(data->block->id, data->block->metadata_id)[data->face];
    primitive->tu0 = attributes->u;
    primitive->tv0 = attributes->v;
    primitive->tu1 = BLOCK_TEXTURE_SIZE * width;
    primitive->tv1 = BLOCK_TEXTURE_SIZE * height;
    // NOTE: cd field is used as a marker to determine if a custom tint exists or not
    const CVECTOR tint = attributes->tint.cd
        ? attributes->tint
        : vec3_rgb(0xFF, 0xFF, 0xFF);
    primitive->r = tint.r;
    primitive->g = tint.g;
    primitive->b = tint.b;
    primitive->light_level = data->light_level;
    return primitive;
}

SVECTOR createVertex(const i32 chunk_origin_x,
                     const i32 chunk_origin_y,
                     const i32 chunk_origin_z,
                     const FaceDirection face_dir,
                     const i32 axis,
                     const fixedi32 axis_offset, // [0, BLOCK_SIZE)
                     const i32 x,
                     const i32 y) {
    assert(axis_offset >= 0);
    assert(axis_offset < BLOCK_SIZE);
    SVECTOR vertex = {0};
#define pos(_x, _y, _z) vec3_i16( \
    (chunk_origin_x + (_x)) * BLOCK_SIZE, \
    (chunk_origin_y - (_y)) * BLOCK_SIZE, \
    (chunk_origin_z + (_z)) * BLOCK_SIZE \
)
    switch (face_dir) {
        case FACE_DIR_DOWN: vertex = pos(x, axis, y); vertex.vy += axis_offset; break;
        case FACE_DIR_UP: vertex = pos(x, axis + 1, y); vertex.vy -= axis_offset; break;
        case FACE_DIR_LEFT: vertex = pos(axis, y, x); vertex.vx += axis_offset; break;
        case FACE_DIR_RIGHT: vertex = pos(axis + 1, y, x); vertex.vx -= axis_offset; break;
        case FACE_DIR_BACK: vertex = pos(x, y, axis); vertex.vz += axis_offset; break;
        case FACE_DIR_FRONT: vertex = pos(x, y, axis + 1); vertex.vz -= axis_offset; break;
    }
#undef pos
    return vertex;
}

void createVertices(Chunk* chunk,
                    MeshPrimitive* primitive,
                    const FaceDirection face_dir,
                    const i32 axis,
                    const fixedi32 axis_offset,
                    const i32 x,
                    const i32 y,
                    const i32 w,
                    const i32 h) {
    Mesh* const mesh = &chunk->mesh.face_meshes[face_dir];
    // Construct vertices relative to chunk mesh bottom left origin
    const i32 chunk_origin_x = chunk->position.vx * CHUNK_SIZE;
    const i32 chunk_origin_y = -chunk->position.vy * CHUNK_SIZE;
    const i32 chunk_origin_z = chunk->position.vz * CHUNK_SIZE;
#define _createVertex(_x, _y) createVertex( \
    chunk_origin_x, \
    chunk_origin_y, \
    chunk_origin_z, \
    face_dir, \
    axis, \
    axis_offset, \
    (_x), \
    (_y) \
)
    const SVECTOR vertices[4] = {
        [0] = _createVertex(x, y),
        [1] = _createVertex(x + w, y),
        [2] = _createVertex(x, y + h),
        [3] = _createVertex(x + w, y + h)
    };
#undef _createVertex
    const INDEX indices = QUAD_INDICES[face_dir];
#define bindVertex(v) *nextRenderAttribute(mesh, p_verts, v, n_verts) = vertices[indices.v]
    bindVertex(v0);
    bindVertex(v1);
    bindVertex(v2);
    bindVertex(v3);
#undef bindVertex
}

void createNormal(ChunkMesh* mesh,
                  MeshPrimitive* primitive,
                  const Block* block,
                  const FaceDirection face_dir) {
    // TODO: When  supporting general block models based on the type
    //       this will need to generate potentially multiple normals
    //       for each set of vertices that make up a face. It will
    //       also need to address models that are not axially aligned
    //       such as sugar cane.
    SVECTOR* norm = nextRenderAttribute(&mesh->face_meshes[face_dir], p_norms, n0, n_norms);
    *norm = FACE_DIRECTION_NORMALS_UNIT[face_dir];
}
