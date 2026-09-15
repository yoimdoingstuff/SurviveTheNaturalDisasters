#ifndef NDS_ENGINE_MESH_H
#define NDS_ENGINE_MESH_H

#include "engine/core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_mesh_vertex {
    float x, y, z;
    float u, v;
} nds_mesh_vertex;

typedef struct nds_mesh {
    nds_mesh_vertex* vertices;
    uint16_t* indices;
    size_t vertex_count;
    size_t index_count;
} nds_mesh;

/* Vertex lines accept x y z and optionally u v. Missing UVs default to 0,0. */
nds_result nds_mesh_load(const char* path, nds_mesh* out_mesh);
nds_result nds_mesh_load_text(const char* text, nds_mesh* out_mesh);
void nds_mesh_destroy(nds_mesh* mesh);

#ifdef __cplusplus
}
#endif

#endif
